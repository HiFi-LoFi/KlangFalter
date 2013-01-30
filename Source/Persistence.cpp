// ==================================================================================
// Copyright (c) 2012 HiFi-LoFi
//
// This is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// ==================================================================================

#include "Persistence.h"

#include "IRAgent.h"
#include "IRManager.h"

#include <algorithm>
#include <list>
#include <map>


namespace internal
{
  
  struct IRAgentConfiguration
  {
    IRAgent* _irAgent;
    String _file;
    int _fileChannel;
    double _stretch;
  };
  
} // End of namespace internal



XmlElement* SaveState(const File& irDirectory, PluginAudioProcessor& processor)
{ 
  ScopedPointer<XmlElement> convolutionElement(new XmlElement("Convolution"));
  convolutionElement->setAttribute("pluginVersion", juce::String(ProjectInfo::versionString));
  convolutionElement->setAttribute("wetOn", processor.getParameter(Parameters::WetOn));
  convolutionElement->setAttribute("wet", processor.getParameter(Parameters::Wet));
  convolutionElement->setAttribute("dryOn", processor.getParameter(Parameters::DryOn));
  convolutionElement->setAttribute("dry", processor.getParameter(Parameters::Dry));
  convolutionElement->setAttribute("autoGainOn", processor.getParameter(Parameters::AutoGainOn));

  convolutionElement->setAttribute("eqLoShelfOn", processor.getParameter(Parameters::EqLowOn));
  convolutionElement->setAttribute("eqLoShelfFreq", processor.getParameter(Parameters::EqLowFreq));
  convolutionElement->setAttribute("eqLoShelfGainDb", processor.getParameter(Parameters::EqLowGainDb));
  convolutionElement->setAttribute("eqLoShelfQ", processor.getParameter(Parameters::EqLowQ));
  
  convolutionElement->setAttribute("eqHiShelfOn", processor.getParameter(Parameters::EqHighOn));
  convolutionElement->setAttribute("eqHiShelfFreq", processor.getParameter(Parameters::EqHighFreq));
  convolutionElement->setAttribute("eqHiShelfGainDb", processor.getParameter(Parameters::EqHighGainDb));
  convolutionElement->setAttribute("eqHiShelfQ", processor.getParameter(Parameters::EqHighQ));
  
  const IRManager& irManager = processor.getIRManager();

  // Parameters
  convolutionElement->setAttribute("fileBeginSeconds", irManager.getFileBeginSeconds());
  convolutionElement->setAttribute("stretch", irManager.getStretch());
  convolutionElement->setAttribute("predelayMs", irManager.getPredelayMs());
  convolutionElement->setAttribute("reverse", irManager.getReverse());
        
  // Envelope
  {
    Envelope envelope = irManager.getEnvelope();
    if (!envelope.isNeutral())
    {
      envelope.setReverse(false);
      XmlElement* envelopeElement = new XmlElement("Envelope");
      envelopeElement->setAttribute("mode", "LinearDb"); // Currently, that's the only supported value, but maybe there's also e.g. spline interpolation at some point in future
      for (size_t i=0; i<envelope.getNodeCount(); ++i)
      {
        XmlElement* envelopeNodeElement = new XmlElement("Node");
        envelopeNodeElement->setAttribute("x", envelope.getX(i));
        envelopeNodeElement->setAttribute("gain", envelope.getY(i));
        envelopeElement->addChildElement(envelopeNodeElement);
      }
      convolutionElement->addChildElement(envelopeElement);
    }
          
    // IRs
    auto irAgents = irManager.getAgents();
    for (auto it=irAgents.begin(); it!=irAgents.end(); ++it)
    {
      IRAgent* irAgent = (*it);
      if (!irAgent)
      {
        continue;
      }
      
      const File irFile = irAgent->getFile();
      if (irFile == File::nonexistent)
      {
        continue;
      }
      
      XmlElement* irElement = new XmlElement("ImpulseResponse");
      irElement->setAttribute("input", static_cast<int>(irAgent->getInputChannel()));
      irElement->setAttribute("output", static_cast<int>(irAgent->getOutputChannel()));
      irElement->setAttribute("file", irFile.getRelativePathFrom(irDirectory));
      irElement->setAttribute("fileChannel", static_cast<int>(irAgent->getFileChannel()));
      convolutionElement->addChildElement(irElement);
    }
  }
  return convolutionElement.release();
}




bool LoadState(const File& irDirectory, XmlElement& element, PluginAudioProcessor& processor)
{  
  if (element.getTagName() != "Convolution")
  {
    return false;
  }  
  IRManager& irManager = processor.getIRManager();

  // Phase 1: Load all data
  bool wetOn = element.getBoolAttribute("wetOn", 1.0);
  double wet = element.getDoubleAttribute("wet", -1.0);
  bool dryOn = element.getBoolAttribute("dryOn", 1.0);
  double dry = element.getDoubleAttribute("dry", -1.0);
  bool autoGainOn = element.getBoolAttribute("autoGainOn", true);
  double fileBeginSeconds = element.getDoubleAttribute("fileBeginSeconds", 0.0);
  double stretch = element.getDoubleAttribute("stretch", 1.0);
  double predelayMs = element.getDoubleAttribute("predelayMs", 0.0);
  bool reverse = element.getBoolAttribute("reverse", false);
  
  bool eqLoShelfOn = element.getBoolAttribute("eqLoShelfOn", false);
  double eqLoShelfFreq = element.getDoubleAttribute("eqLoShelfFreq", 20.0);
  double eqLoShelfGainDb = element.getDoubleAttribute("eqLoShelfGainDb", 0.0);
  double eqLoShelfQ = element.getDoubleAttribute("eqLoShelfQ", 1.0);
  
  
  bool eqHiShelfOn = element.getBoolAttribute("eqHiShelfOn", false);
  double eqHiShelfFreq = element.getDoubleAttribute("eqHiShelfFreq", 20000.0);
  double eqHiShelfGainDb = element.getDoubleAttribute("eqHiShelfGainDb", 0.0);
  double eqHiShelfQ = element.getDoubleAttribute("eqHiShelfQ", 1.0);
  
  if (wet < 0.0 || wet > 1.0)
  {
    return false;
  }
  if (dry < 0.0 || dry > 1.0)
  {
    return false;
  }
  
  Envelope envelope;
  XmlElement* envelopeElement = element.getChildByName("Envelope");
  if (envelopeElement)
  {
    std::vector<std::pair<double, double>> envelopeValues;
    forEachXmlChildElementWithTagName (*envelopeElement, envelopeNodeElement, "Node")
    {
      const double x = envelopeNodeElement->getDoubleAttribute("x", -1.0);
      const double y = envelopeNodeElement->getDoubleAttribute("gain", -1.0);
      if (x < -0.00001 || y < -0.00001)
      {
        return false;
      }
      envelopeValues.push_back(std::make_pair(std::min(1.0, std::max(0.0, x)), std::min(1.0, std::max(0.0, y))));
    }
    if (envelopeValues.size() < 2)
    {
      return false;
    }
    std::sort(envelopeValues.begin(), envelopeValues.end());
    Envelope loaded(envelopeValues.front().second, envelopeValues.back().second);
    for (size_t i=1; i<envelopeValues.size()-1; ++i)
    {
      loaded.insertNode(envelopeValues[i].first, envelopeValues[i].second);
    }
    loaded.setReverse(reverse);
    envelope = loaded;
  }
  
  // IRs
  std::vector<internal::IRAgentConfiguration> irConfigurations;
  forEachXmlChildElementWithTagName (element, irElement, "ImpulseResponse")
  {
    const int inputChannel = irElement->getIntAttribute("input", -1);
    const int outputChannel = irElement->getIntAttribute("output", -1);
    if (inputChannel == -1 || outputChannel == -1)
    {
      return false;
    }
    
    IRAgent* irAgent = irManager.getAgent(inputChannel, outputChannel);
    if (!irAgent)
    {
      return false;
    }
    internal::IRAgentConfiguration configuration;
    configuration._irAgent = irAgent;
    configuration._file = irElement->getStringAttribute("file", String::empty);
    configuration._fileChannel = irElement->getIntAttribute("fileChannel", -1);
    irConfigurations.push_back(configuration);
  }
  
  // Phase 2: Restore the state
  irManager.reset();
  
  processor.setParameterNotifyingHost(Parameters::WetOn, wetOn);
  processor.setParameterNotifyingHost(Parameters::Wet, static_cast<float>(wet));
  processor.setParameterNotifyingHost(Parameters::DryOn, dryOn);
  processor.setParameterNotifyingHost(Parameters::Dry, static_cast<float>(dry));
  processor.setParameterNotifyingHost(Parameters::AutoGainOn, autoGainOn);
  
  processor.setParameterNotifyingHost(Parameters::EqLowOn, eqLoShelfOn);
  processor.setParameterNotifyingHost(Parameters::EqLowFreq, static_cast<float>(eqLoShelfFreq));
  processor.setParameterNotifyingHost(Parameters::EqLowGainDb, static_cast<float>(eqLoShelfGainDb));
  processor.setParameterNotifyingHost(Parameters::EqLowQ, static_cast<float>(eqLoShelfQ));
  
  processor.setParameterNotifyingHost(Parameters::EqHighOn, eqHiShelfOn);
  processor.setParameterNotifyingHost(Parameters::EqHighFreq, static_cast<float>(eqHiShelfFreq));
  processor.setParameterNotifyingHost(Parameters::EqHighGainDb, static_cast<float>(eqHiShelfGainDb));
  processor.setParameterNotifyingHost(Parameters::EqHighQ, static_cast<float>(eqHiShelfQ));
  
  irManager.setFileBeginSeconds(fileBeginSeconds);
  irManager.setPredelayMs(predelayMs);
  irManager.setStretch(stretch);
  irManager.setReverse(reverse);
  irManager.setEnvelope(envelope);
  
  for (auto it=irConfigurations.begin(); it!=irConfigurations.end(); ++it)
  {
    IRAgent* irAgent = it->_irAgent;
    const File irFile = irDirectory.getChildFile(it->_file);
    irAgent->setFile(irFile, it->_fileChannel);
  }
  
  return true;
}

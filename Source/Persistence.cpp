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
#include "Parameters.h"

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


  static juce::String EqType2String(Parameters::EqType eqType)
  {
    if (eqType == Parameters::Cut)
    {
      return juce::String("Cut");
    }
    if (eqType == Parameters::Shelf)
    {
      return juce::String("Shelf");
    }
    return juce::String();
  }

  static Parameters::EqType String2EqType(const juce::String& eqTypeString)
  {
    if (eqTypeString == juce::String("Cut"))
    {
      return Parameters::Cut;
    }
    if (eqTypeString == juce::String("Shelf"))
    {
      return Parameters::Shelf;
    }
    return Parameters::Cut;
  }
  
} // End of namespace internal



XmlElement* SaveState(const File& irDirectory, Processor& processor)
{ 
  ScopedPointer<XmlElement> convolutionElement(new XmlElement("Convolution"));
  convolutionElement->setAttribute("pluginVersion", juce::String(ProjectInfo::versionString));
  convolutionElement->setAttribute("wetOn", processor.getParameter(Parameters::WetOn));
  convolutionElement->setAttribute("wetDecibels", processor.getParameter(Parameters::WetDecibels));
  convolutionElement->setAttribute("dryOn", processor.getParameter(Parameters::DryOn));
  convolutionElement->setAttribute("dryDecibels", processor.getParameter(Parameters::DryDecibels));
  convolutionElement->setAttribute("autoGainOn", processor.getParameter(Parameters::AutoGainOn));
  convolutionElement->setAttribute("eqLowType", internal::EqType2String(Parameters::EqType(processor.getParameter(Parameters::EqLowType))));
  convolutionElement->setAttribute("eqLowCutFreq", processor.getParameter(Parameters::EqLowCutFreq));
  convolutionElement->setAttribute("eqLowShelfFreq", processor.getParameter(Parameters::EqLowShelfFreq));
  convolutionElement->setAttribute("eqLowShelfDecibels", processor.getParameter(Parameters::EqLowShelfDecibels));
  convolutionElement->setAttribute("eqHighType", internal::EqType2String(Parameters::EqType(processor.getParameter(Parameters::EqHighType))));
  convolutionElement->setAttribute("eqHighCutFreq", processor.getParameter(Parameters::EqHighCutFreq));
  convolutionElement->setAttribute("eqHighShelfFreq", processor.getParameter(Parameters::EqHighShelfFreq));
  convolutionElement->setAttribute("eqHighShelfDecibels", processor.getParameter(Parameters::EqHighShelfDecibels));
  convolutionElement->setAttribute("fileBeginSeconds", processor.getFileBeginSeconds());
  convolutionElement->setAttribute("stretch", processor.getStretch());
  convolutionElement->setAttribute("predelayMs", processor.getPredelayMs());
  convolutionElement->setAttribute("stereoWidth", processor.getParameter(Parameters::StereoWidth));
  convolutionElement->setAttribute("reverse", processor.getReverse());
        
  // Envelope
  {
    Envelope envelope = processor.getEnvelope();
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
    auto irAgents = processor.getAgents();
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




bool LoadState(const File& irDirectory, XmlElement& element, Processor& processor)
{  
  if (element.getTagName() != "Convolution")
  {
    return false;
  }

  // Phase 1: Load all data
  bool wetOn = element.getBoolAttribute("wetOn", Parameters::WetOn.getDefaultValue());
  double wetDecibels = element.getDoubleAttribute("wetDecibels", Parameters::WetDecibels.getDefaultValue());
  bool dryOn = element.getBoolAttribute("dryOn", Parameters::DryOn.getDefaultValue());
  double dryDecibels = element.getDoubleAttribute("dryDecibels", Parameters::DryDecibels.getDefaultValue());
  bool autoGainOn = element.getBoolAttribute("autoGainOn", Parameters::AutoGainOn.getDefaultValue());
  double fileBeginSeconds = element.getDoubleAttribute("fileBeginSeconds", 0.0);
  double stretch = element.getDoubleAttribute("stretch", 1.0);
  double predelayMs = element.getDoubleAttribute("predelayMs", 0.0);
  double stereoWidth = element.getDoubleAttribute("stereoWidth", Parameters::StereoWidth.getDefaultValue());
  bool reverse = element.getBoolAttribute("reverse", false);
  
  Parameters::EqType eqLoType = internal::String2EqType(element.getStringAttribute("eqLowType", juce::String()));
  double eqLoCutFreq = element.getDoubleAttribute("eqLowCutFreq", Parameters::EqLowCutFreq.getDefaultValue());
  double eqLoShelfFreq = element.getDoubleAttribute("eqLowShelfFreq", Parameters::EqLowShelfFreq.getDefaultValue());
  double eqLoShelfDecibels = element.getDoubleAttribute("eqLowShelfDecibels", Parameters::EqLowShelfDecibels.getDefaultValue());

  Parameters::EqType eqHiType = internal::String2EqType(element.getStringAttribute("eqHighType", juce::String()));
  double eqHiCutFreq = element.getDoubleAttribute("eqHighCutFreq", Parameters::EqHighCutFreq.getDefaultValue());
  double eqHiShelfFreq = element.getDoubleAttribute("eqHighShelfFreq", Parameters::EqHighShelfFreq.getDefaultValue());
  double eqHiShelfDecibels = element.getDoubleAttribute("eqHighShelfDecibels", Parameters::EqHighShelfDecibels.getDefaultValue());
  
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
    
    IRAgent* irAgent = processor.getAgent(inputChannel, outputChannel);
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
  processor.clearConvolvers();  
  processor.setParameterNotifyingHost(Parameters::WetOn, wetOn);
  processor.setParameterNotifyingHost(Parameters::WetDecibels, static_cast<float>(wetDecibels));
  processor.setParameterNotifyingHost(Parameters::DryOn, dryOn);
  processor.setParameterNotifyingHost(Parameters::DryDecibels, static_cast<float>(dryDecibels));
  processor.setParameterNotifyingHost(Parameters::AutoGainOn, autoGainOn);  
  processor.setParameterNotifyingHost(Parameters::EqLowType, static_cast<int>(eqLoType));
  processor.setParameterNotifyingHost(Parameters::EqLowCutFreq, static_cast<float>(eqLoCutFreq));
  processor.setParameterNotifyingHost(Parameters::EqLowShelfFreq, static_cast<float>(eqLoShelfFreq));
  processor.setParameterNotifyingHost(Parameters::EqLowShelfDecibels, static_cast<float>(eqLoShelfDecibels));
  processor.setParameterNotifyingHost(Parameters::EqHighType, static_cast<int>(eqHiType));
  processor.setParameterNotifyingHost(Parameters::EqHighCutFreq, static_cast<float>(eqHiCutFreq));
  processor.setParameterNotifyingHost(Parameters::EqHighShelfFreq, static_cast<float>(eqHiShelfFreq));
  processor.setParameterNotifyingHost(Parameters::EqHighShelfDecibels, static_cast<float>(eqHiShelfDecibels));  
  processor.setParameterNotifyingHost(Parameters::StereoWidth, static_cast<float>(stereoWidth));
  processor.setFileBeginSeconds(fileBeginSeconds);
  processor.setPredelayMs(predelayMs);  
  processor.setStretch(stretch);
  processor.setReverse(reverse);
  processor.setEnvelope(envelope);  
  for (auto it=irConfigurations.begin(); it!=irConfigurations.end(); ++it)
  {
    IRAgent* irAgent = it->_irAgent;
    const File irFile = irDirectory.getChildFile(it->_file);
    irAgent->setFile(irFile, it->_fileChannel);
  }  
  return true;
}

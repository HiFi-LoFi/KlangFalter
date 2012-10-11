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

#include "Settings.h"


Settings::Settings() :
  _properties()
{
  juce::PropertiesFile::Options fileOptions;
  fileOptions.applicationName = "KlangFalter";
  fileOptions.filenameSuffix = "settings";
  fileOptions.folderName = "KlangFalter";
  fileOptions.osxLibrarySubFolder = "Application Support"; // Recommended by Apple resp. the Juce documentation
  fileOptions.commonToAllUsers = false;
  fileOptions.ignoreCaseOfKeyNames = false;
  fileOptions.storageFormat = juce::PropertiesFile::storeAsXML;
  _properties.setStorageParameters(fileOptions);
}


Settings::~Settings()
{
  _properties.closeFiles();
}


void Settings::addChangeListener(juce::ChangeListener* listener)
{
  juce::PropertiesFile* propertiesFile = _properties.getUserSettings();
  if (propertiesFile)
  {
    propertiesFile->addChangeListener(listener);
  }
}


void Settings::removeChangeListener(juce::ChangeListener* listener)
{
  juce::PropertiesFile* propertiesFile = _properties.getUserSettings();
  if (propertiesFile)
  {
    propertiesFile->removeChangeListener(listener);
  }
}


size_t Settings::getConvolverBlockSize()
{
  size_t blockSize = 4096;
  juce::PropertiesFile* propertiesFile = _properties.getUserSettings();
  if (propertiesFile)
  {
    blockSize = propertiesFile->getIntValue("ConvolverBlockSize", blockSize);
  }
  return blockSize;
}


void Settings::setConvolverBlockSize(size_t blockSize)
{
  juce::PropertiesFile* propertiesFile = _properties.getUserSettings();
  if (propertiesFile)
  {
    propertiesFile->setValue("ConvolverBlockSize", static_cast<int>(blockSize));
    propertiesFile->saveIfNeeded();
  }
}


juce::File Settings::getImpulseResponseDirectory()
{
  juce::PropertiesFile* propertiesFile = _properties.getUserSettings();
  if (propertiesFile)
  {
    const juce::File dir = juce::File::createFileWithoutCheckingPath(propertiesFile->getValue("ImpulseResponseDirectory", juce::String()));
    if (dir.exists() && dir.isDirectory())
    {
      return dir;
    }
  }
  return juce::File::nonexistent;
}


void Settings::setImpulseResponseDirectory(const juce::File& directory)
{
  if (directory.exists() && directory.isDirectory())
  {
    juce::PropertiesFile* propertiesFile = _properties.getUserSettings();
    if (propertiesFile)
    {
      propertiesFile->setValue("ImpulseResponseDirectory", directory.getFullPathName());
      propertiesFile->saveIfNeeded();
    }
  }
}

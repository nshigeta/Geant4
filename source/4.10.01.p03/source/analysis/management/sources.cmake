#------------------------------------------------------------------------------
# sources.cmake
# Module : G4csv
# Package: Geant4.src.G4analysis.G4analysismng
#
# Sources description for a library.
# Lists the sources and headers of the code explicitely.
# Lists include paths needed.
# Lists the internal granular and global dependencies of the library.
# Source specific properties should be added at the end.
#
# Generated on : 15/07/2013
#
# $Id: sources.cmake 83748 2014-09-12 12:13:37Z gcosmo $
#
#------------------------------------------------------------------------------

# List external includes needed.
include_directories(${CLHEP_INCLUDE_DIRS})

# List internal includes needed.
include_directories(${CMAKE_SOURCE_DIR}/source/global/management/include)
include_directories(${CMAKE_SOURCE_DIR}/source/intercoms/include)
include_directories(${CMAKE_SOURCE_DIR}/source/analysis/g4tools/include)

#
# Define the Geant4 Module.
#
include(Geant4MacroDefineModule)
GEANT4_DEFINE_MODULE(NAME G4analysismng
    HEADERS
        G4AnalysisVerbose.hh
        G4AnalysisManagerState.hh
        G4AnalysisMessenger.hh
        G4AnalysisUtilities.hh
        G4BaseAnalysisManager.hh
        G4BaseFileManager.hh
        G4BinScheme.hh
        G4Fcn.hh
        G4FileMessenger.hh
        G4H1Messenger.hh
        G4H2Messenger.hh
        G4H3Messenger.hh
        G4P1Messenger.hh
        G4P2Messenger.hh
        G4HnInformation.hh
        G4HnManager.hh
        G4HnMessenger.hh
        G4VAnalysisManager.hh
        G4VAnalysisManager.icc
        G4VAnalysisReader.hh
        G4VAnalysisReader.icc
        G4VFileManager.hh
        G4VH1Manager.hh
        G4VH2Manager.hh
        G4VH3Manager.hh
        G4VNtupleManager.hh
        G4VP1Manager.hh
        G4VP2Manager.hh
        G4VRNtupleManager.hh
        g4analysis_defs.hh
    SOURCES
        G4AnalysisVerbose.cc
        G4AnalysisManagerState.cc
        G4AnalysisMessenger.cc
        G4BaseAnalysisManager.cc
        G4BaseFileManager.cc
        G4AnalysisUtilities.cc
        G4BinScheme.cc
        G4Fcn.cc
        G4FileMessenger.cc
        G4H1Messenger.cc
        G4H2Messenger.cc
        G4H3Messenger.cc
        G4P1Messenger.cc
        G4P2Messenger.cc
        G4HnManager.cc
        G4HnMessenger.cc
        G4VAnalysisManager.cc
        G4VAnalysisReader.cc
        G4VFileManager.cc
        G4VH1Manager.cc
        G4VH2Manager.cc
        G4VH3Manager.cc
        G4VNtupleManager.cc
        G4VP1Manager.cc
        G4VP2Manager.cc
    GRANULAR_DEPENDENCIES
        G4globman
        G4intercoms
    GLOBAL_DEPENDENCIES
        G4global
        G4intercoms
    LINK_LIBRARIES
)

# List any source specific properties here

##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=wrap
ConfigurationName      :=Release
WorkspacePath          :=/home/utente/Documenti/squid_wrap
ProjectPath            :=/home/utente/Documenti/squid_wrap/wrap
IntermediateDirectory  :=./Release
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=utente
Date                   :=08/02/20
CodeLitePath           :=/home/utente/.codelite
LinkerName             :=/usr/bin/g++
SharedObjectLinkerName :=/usr/bin/g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=$(PreprocessorSwitch)NDEBUG 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="wrap.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++
CC       := /usr/bin/gcc
CXXFLAGS :=  -O2 -Wall $(Preprocessors)
CFLAGS   :=  -O2 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IntermediateDirectory)/pLanConn.cpp$(ObjectSuffix) $(IntermediateDirectory)/base64.cpp$(ObjectSuffix) $(IntermediateDirectory)/SocketUtil.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@test -d ./Release || $(MakeDirCommand) ./Release


$(IntermediateDirectory)/.d:
	@test -d ./Release || $(MakeDirCommand) ./Release

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/main.cpp$(ObjectSuffix): main.cpp $(IntermediateDirectory)/main.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/utente/Documenti/squid_wrap/wrap/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main.cpp$(DependSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/main.cpp$(DependSuffix) -MM main.cpp

$(IntermediateDirectory)/main.cpp$(PreprocessSuffix): main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main.cpp$(PreprocessSuffix) main.cpp

$(IntermediateDirectory)/pLanConn.cpp$(ObjectSuffix): pLanConn.cpp $(IntermediateDirectory)/pLanConn.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/utente/Documenti/squid_wrap/wrap/pLanConn.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/pLanConn.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/pLanConn.cpp$(DependSuffix): pLanConn.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/pLanConn.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/pLanConn.cpp$(DependSuffix) -MM pLanConn.cpp

$(IntermediateDirectory)/pLanConn.cpp$(PreprocessSuffix): pLanConn.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/pLanConn.cpp$(PreprocessSuffix) pLanConn.cpp

$(IntermediateDirectory)/base64.cpp$(ObjectSuffix): base64.cpp $(IntermediateDirectory)/base64.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/utente/Documenti/squid_wrap/wrap/base64.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/base64.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/base64.cpp$(DependSuffix): base64.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/base64.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/base64.cpp$(DependSuffix) -MM base64.cpp

$(IntermediateDirectory)/base64.cpp$(PreprocessSuffix): base64.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/base64.cpp$(PreprocessSuffix) base64.cpp

$(IntermediateDirectory)/SocketUtil.cpp$(ObjectSuffix): SocketUtil.cpp $(IntermediateDirectory)/SocketUtil.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/utente/Documenti/squid_wrap/wrap/SocketUtil.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/SocketUtil.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/SocketUtil.cpp$(DependSuffix): SocketUtil.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/SocketUtil.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/SocketUtil.cpp$(DependSuffix) -MM SocketUtil.cpp

$(IntermediateDirectory)/SocketUtil.cpp$(PreprocessSuffix): SocketUtil.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/SocketUtil.cpp$(PreprocessSuffix) SocketUtil.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Release/



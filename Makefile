CC=g++
OBJS=Data/Config.o Data/Utils.o Data/DataReader.o Data/WavSource.o Data/PushSource.o Data/MicSource.o Data/FileIO.o \
Features/Coeffs.o Features/Filter.o Features/Frame.o Features/Transform.o Features/Feature.o \
Search/AcousticScorer.o Search/Token.o Search/Search.o \
Network/HTKAcousticModel.o Network/Dictionary.o Network/FSTAssembly.o
LD=-lportaudio
TARGET=Ear
COMPILER=Compile

all: $(OBJS) Ear.o Compile.o
	g++ -O6 $(OBJS) Ear.o $(LD) -o $(TARGET)
	g++ -O6 $(OBJS) Compile.o $(LD) -o $(COMPILER)

clean:
	-rm Data/*.o
	-rm Features/*.o
	-rm Search/*.o
	-rm Network/*.o
	-rm *.o
	-rm $(TARGET)
	-rm $(COMPILER)
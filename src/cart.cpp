#include <iostream>
#include <fstream>
#include <vector>

#include <boost/format.hpp>

#include "../include/cart.h"


// CONSTRUCTOR
Cart::Cart(char* filepath, Logger& newLogger)
    : logger(newLogger)
{   
    boost::format fmt = boost::format("Filename loading: %s") % filepath;
    std::string msg = fmt.str();

    // open file stream for rom & extract header data
    logger << Logger::logType::LOG_INFO
        << msg
        << Logger::logType::LOG_ENDLINE;
    std::ifstream rom(filepath);
    getHeaderData(rom);
    getRomData(rom);
    getMapper();
}


Cart::~Cart(){
    
}


// reads from the cartridge
u8 Cart::read(u16 address){
    mapper->getMappedAddress(address);
    return prgRom[address];
}


void Cart::write(u16 address, u8 data){
    mapper->getMappedAddress(address);
    // TODO: writing support for on-cart RAM
    // for now, do nothing
}


u8 Cart::readPPU(u16 address){
    // kinda hacky temporary solution for getting nametables 
    // to screen. Should be using Mapper for mapping addresses
    return chrRom[address];
}


void Cart::writePPU(u16 address, u8 data){
    // Probably not needed, but boilerplate here 
    // just in case
    // TODO: Figure out if I need this method
}


// gets the header data from the rom stream
void Cart::getHeaderData(std::ifstream &ifs){
    char buffer[NES_HEADER_SIZE];
    ifs.read(buffer, NES_HEADER_SIZE);

    // read buffer into header vector
    header = std::vector<u8>(NES_HEADER_SIZE);
    for (int i = 0; i != NES_HEADER_SIZE; i++){
        header[i] = buffer[i];
    }
}


// Gets the mapperID from the header data & creates a pointer to the needed mapper
void Cart::getMapper(){
    u8 mapperID = (header[6] & 0xF0) >> 4 | (header[7] & 0xF0);

    switch (mapperID){
        case 0: mapper = std::make_unique<Mapper000>(header[4], header[5]); break;
        default: std::cout << "FUCK THAT ROM" << std::endl; break;
    }
}


// gets prgRom, chrRom, etc.. from the rom stream
void Cart::getRomData(std::ifstream &ifs){
    mirroring = (header[6] & 0x01) ? VERTICAL_MIRROR : HORIZONTAL_MIRROR;
    if (header[6] & 0x04) ifs.seekg(512, std::ios_base::cur);

    prgRomSize = header[4] * 16384;
    prgRom.resize(prgRomSize);
    ifs.read((char*)prgRom.data(), prgRom.size());

    chrRomSize = header[5] * 8192;
    chrRom.resize(chrRomSize);
    ifs.read((char*)chrRom.data(), chrRom.size());
}
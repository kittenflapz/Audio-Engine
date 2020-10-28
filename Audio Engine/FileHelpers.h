#pragma once
#include <iostream>
#include <fstream>


bool fileExists(string filename)
{
    std::ifstream ifile(filename);
    return !ifile.fail();
}
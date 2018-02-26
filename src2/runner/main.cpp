#include <iostream>
#include <string>
#include "Option.h"

using namespace std;

int main(int argc, char* argv[]){
    Option opt;
    if(!opt.parseInput(argc, argv)){
        cerr<<"Failed in parsing arguments"<<endl;
        return 1;
    }


    return 0;
}

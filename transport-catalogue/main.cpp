#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    using namespace reader::input;
    using namespace reader::stat;
    using namespace catalogue;
    TransportCatalogue catalogue;
    {
        InputReader reader;
        reader.ReadInput(cin);
        reader.ApplyCommands(catalogue);
    }
    ParseAndPrintStat(catalogue, cin, cout);
}

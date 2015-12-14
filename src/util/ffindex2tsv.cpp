#include <cstdio>
#include "Parameters.h"

#include "DBReader.h"
#include "Debug.h"
#include "Util.h"


int createtsv (int argc, const char * argv[])
{
    std::string usage("Converts a ffindex database to tsv\n");
    usage.append("Written by Martin Steinegger (Martin.Steinegger@campus.lmu.de) & Maria Hauser (mhauser@genzentrum.lmu.de).\n\n");
    usage.append("USAGE: <queryDB> <targetDB> <resultDB> <tsvFile>\n");
    Parameters par;
    par.parseParameters(argc, argv, usage, par.onlyverbosity, 4);

    Debug(Debug::WARNING) << "Query file is " <<  par.db1 << "\n";
    std::string queryHeaderDB =  par.db1 + "_h";
    DBReader querydb_header(queryHeaderDB.c_str(), std::string(queryHeaderDB+".index").c_str());
    querydb_header.open(DBReader::NOSORT);

    Debug(Debug::WARNING) << "Target file is " << par.db2 << "\n";
    std::string targetHeaderDB =  par.db2 + "_h";
    DBReader targetdb_header(targetHeaderDB.c_str(), std::string(targetHeaderDB+".index").c_str());
    targetdb_header.open(DBReader::NOSORT);

    Debug(Debug::WARNING) << "Data file is " << par.db3 << "\n";
    DBReader dbr_data( par.db3.c_str(), std::string( par.db3+".index").c_str());
    dbr_data.open(DBReader::NOSORT);

    FILE *tsvFP =  fopen(par.db4.c_str(), "w");
    char newline[] = {'\n'};
    Debug(Debug::WARNING) << "Start writing file to " << par.db4 << "\n";
    char * dbKey = new char[par.maxSeqLen + 1];

    for(size_t i = 0; i < dbr_data.getSize(); i++){
        std::string queryKey = dbr_data.getDbKey(i);

        char * header_data = querydb_header.getDataByDBKey(queryKey.c_str());
        std::string queryHeader = Util::parseFastaHeader(header_data);


        // write data
        char * data = dbr_data.getData(i);

        while(*data != '\0') {
            Util::parseKey(data, dbKey);
            size_t keyLen = strlen(dbKey);

            char * header_data = targetdb_header.getDataByDBKey(dbKey);
            std::string parsedDbkey = Util::parseFastaHeader(header_data);
            char * nextLine = Util::skipLine(data);
            // write to file
            fwrite(queryHeader.c_str(), sizeof(char), queryHeader.length(), tsvFP);
            fwrite("\t", sizeof(char), 1, tsvFP);
            fwrite(parsedDbkey.c_str(), sizeof(char), parsedDbkey.length(), tsvFP);
            fwrite("\t", sizeof(char), 1, tsvFP);
            fwrite(data + keyLen, sizeof(char), (nextLine - (data + keyLen)) - 1, tsvFP );
            fwrite("\n", sizeof(char), 1, tsvFP);

            data = nextLine;
        }
        //fwrite(newline, sizeof(char), 1, tsvFP);

    }
    delete [] dbKey;
    Debug(Debug::WARNING) << "Done." << "\n";

    fclose(tsvFP);
    targetdb_header.close();
    querydb_header.close();
    dbr_data.close();


    return 0;
}
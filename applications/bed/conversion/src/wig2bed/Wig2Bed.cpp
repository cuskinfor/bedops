/*
  FILE: Wig2Bed.cpp
  AUTHOR: Scott Kuehn, Shane Neph
  CREATE DATE: Tue Oct 30 11:14:40 PDT 2007
  PROJECT: utility
  ID: $Id$
*/

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>

#include "suite/BEDOPS.Constants.hpp"
#include "suite/BEDOPS.Version.hpp"

#include "Input.hpp"


namespace {
  const std::string prognm = "wig2bed";
  const std::string citation = BEDOPS::citation();
  const std::string version = BEDOPS::revision();
  const std::string authors = "Scott Kuehn & Shane Neph";
} // unnamed


int
main(int argc, char **argv)
{
  bool iserror = true;
  try
    {
      Wig2Bed::Input progInput(argc, argv);
      bool multiout = !progInput.basename_.empty();
      int cntr = 1;
      bool startWrite = false;
      FILE* outfile = stdout;

      for(Wig2Bed::Input::InFileIterator i = progInput.inFiles_.begin(); i != progInput.inFiles_.end(); i++)      
        {
          if ( multiout ) {
            std::stringstream con;
            con << progInput.basename_ << "." << cntr++;
            std::string s = con.str();
            outfile = std::fopen(s.c_str(), "w");
          }
          Bed::LineCountType line = 0;
          Bed::LineCountType posLines = 0;
          int span;
          int step;
          Bed::CoordType startPos;
          Bed::CoordType endPos;
          float score;
          char chromBuf[20];
          bool isFixedStep = false;
          std::string currLine;
          startWrite = false;

          while(std::getline(**i, currLine))
            {
              line++;

              /*Ignore leading white space and comments*/
              std::size_t lineStartPos = currLine.find_first_not_of(" \t");
              if ( lineStartPos == std::string::npos || currLine[lineStartPos] == '#' ) {
                if ( multiout && startWrite ) {
                  startWrite = false;
                  std::fclose(outfile);
                  std::stringstream con;
                  con << progInput.basename_ << "." << cntr++;
                  std::string s = con.str();
                  outfile = std::fopen(s.c_str(), "w");
                }
                continue;
              }
              currLine.assign(currLine, lineStartPos, currLine.size() - lineStartPos);

              /* Evaluate Wig Line */
              if ( (std::strncmp(currLine.c_str(), "track", 5) == 0) || std::strncmp(currLine.c_str(), "browser", 7) == 0 ) {
                if ( multiout && startWrite ) {
                  startWrite = false;
                  std::fclose(outfile);
                  std::stringstream con;
                  con << progInput.basename_ << "." << cntr++;
                  std::string s = con.str();
                  outfile = std::fopen(s.c_str(), "w");
                }
                continue;
              }
              else if (std::strncmp(currLine.c_str(), "variableStep", 12) == 0) 
                {
                  int fields = std::sscanf(currLine.c_str(), "variableStep chrom=%s span=%d\n", chromBuf, &span);
                  if (fields < 1) 
                    {
                      std::stringstream err;
                      err << "Invalid variableStep header on line " << line << std::endl;
                      throw(err.str());
                    }
                  if (fields == 1) 
                    span = 1;
                  isFixedStep = false;
                  if ( multiout && startWrite ) {
                    startWrite = false;
                    std::fclose(outfile);
                    std::stringstream con;
                    con << progInput.basename_ << "." << cntr++;
                    std::string s = con.str();
                    outfile = std::fopen(s.c_str(), "w");
                  }
                }
              else if (std::strncmp(currLine.c_str(), "fixedStep", 9) == 0) 
                {
                  int fields = std::sscanf(currLine.c_str(), "fixedStep chrom=%s start=%" SCNu64 " step=%d span=%d\n", chromBuf, 
                                           &startPos, &step, &span);
                  if (fields < 3) 
                    {
                      std::stringstream err;
                      err << "Invalid fixedStep header on line " << line << std::endl;
                      throw(err.str());
                    }
                  if(fields == 3)
                    span = 1;
                  isFixedStep = true;
                  if ( multiout && startWrite ) {
                    startWrite = false;
                    std::fclose(outfile);
                    std::stringstream con;
                    con << progInput.basename_ << "." << cntr++;
                    std::string s = con.str();
                    outfile = std::fopen(s.c_str(), "w");
                  }
                }
              else if (std::strncmp(currLine.c_str(), "chr", 3) == 0) 
                {
                  /*Bed Format*/
                  posLines++;
                  int fields = std::sscanf(currLine.c_str(), "%s\t%" SCNu64 "\t%" SCNu64 "\t%f\n", chromBuf, 
                                           &startPos, &endPos, &score);
                  if(fields != 4)
                    {
                      std::stringstream err;
                      err << "Invalid wig line: " << line << std::endl;
                      throw(err.str());
                    }
                  std::fprintf(outfile, "%s\t%" PRIu64 "\t%" PRIu64 "\tid-%" PRIu64 "\t%f\n", chromBuf, startPos-1, endPos-1, posLines, score);
                  startWrite = true;
                }
              else
                {
                  /* Data Value */
                  if(isFixedStep)
                    {
                      int fields = std::sscanf(currLine.c_str(), "%f\n", &score);
                      if(fields != 1)
                        {
                          std::stringstream err;
                          err << "Invalid wig line: " << line << std::endl;
                          throw(err.str());
                        }
                      posLines++;
                      std::fprintf(outfile, "%s\t%" PRIu64 "\t%" PRIu64 "\tid-%" PRIu64 "\t%f\n", chromBuf, startPos-1, startPos+span-1, posLines, score);
                      startPos += step;
                      startWrite = true;
                    }
                  else
                    {
                      /* Variable step*/
                      int fields = std::sscanf(currLine.c_str(), "%" SCNu64 "\t%f\n", &startPos, &score);
                      if(fields !=2)
                        {
                          std::stringstream err;
                          err << "Invalid wig line: " << line << std::endl;
                          throw(err.str());
                        }
                      posLines++;
                      std::fprintf(outfile, "%s\t%" PRIu64 "\t%" PRIu64 "\tid-%" PRIu64 "\t%f\n", chromBuf, startPos-1, startPos+span-1, posLines, score);
                      startWrite = true;
                    }
                }
            }
        }
      iserror = false;
      if ( multiout && startWrite )
        std::fclose(outfile);
    }
  catch(Wig2Bed::Help h)
    {
      std::cout << prognm << std::endl;
      std::cout << "  citation: " + citation << std::endl;
      std::cout << "  version:  " + version << std::endl;
      std::cout << "  authors:  " + authors << std::endl;
      std::cout << std::endl << Wig2Bed::Input::Usage() << std::endl << std::endl;
      iserror = false;
    }
  catch(const std::string& msg)
    {
      std::cerr << prognm << std::endl;
      std::cerr << "  citation: " << citation << std::endl;
      std::cerr << "  version:  " + version << std::endl;
      std::cerr << "  authors:  " + authors << std::endl;
      std::cerr << std::endl << Wig2Bed::Input::Usage() << std::endl << std::endl;
      std::cerr << "  " << msg << std::endl;
      iserror = true;
    }
  catch(const std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  catch(...)
    {
      std::cerr << "Error:  Aborting" << std::endl;
    }

  if ( iserror )
    return(EXIT_FAILURE);
  return(EXIT_SUCCESS);
}

/** Filename:  LandmarkConverter.cpp
 *
 *  @author Christopher Guy
 *  @version 1.2.0 3/20/17
 *
 *  Previous versions:
 *  0.9.0     CLG     Initial build
 *  0.9.1     CLG     Fixed point-pair read bugs
 *                    Now require output directory to be specified
 *  0.9.2     CLG     No longer require mhd file as input
 *                    Now require specification of output file type
 *  0.9.3     CLG     Minor change so correct .mhd file path is used
 *  1.0.0     CLG     Added greater functionality. Can now handle various input
 *                    and output file types, including 3D Slicer fiducial files
 *  1.1.0     CLG     Fixed issue where landmark order was reversed in output
 *  1.1.1     CLG     Made output text files compatible with Transformix
 *  1.2.0     CLG     Fixed physical/voxel coordinate conversion error
 *
 *  Purpose:
 *  This program reformats landmark pairs from one input format (i.e. iX's 
 *  Matching Points Annotator output) to another (i.e. Transformix's landmark-
 *  based transform parameter file).
 * 
 *  The program is called along with three parameters:
 *  -in_file  The input file containing the landmarks
 *  -in_type  The type of input file from which landmarks will be read:
 *               ix_pp - Point pair file of landmarks match with Image eXplorer
 *                ireg - Registration landmarks from Caliper registration code.
 *  -out_dir  The path of the directory where the output file will be written
 *  -out_type The type of output file to be generated, options include:
 *               tfx_lmk  - Transformix landmark-based transform input file
 *               slr_fid  - 3D Slicer fiducial file
 *               std_txt  - Standard plain text file
 *  -keep_all Whether to keep (1) or discard (0) points marked as 'very unsure'
 *  The files are then read, storing necessary data to be included in the
 *  output file for Transformix. The output file is then written to the
 *  directory specified.
 *
 *  TODO:
 *  Provide greater functionality by utilizing more meta information to allow
 *  for differing voxel dimensions, voxel spacing, origins, etc. between scans
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <stdexcept>

using namespace std;

// Landmarks structure is defined
struct LandmarkPairs
{
    int numPoints;
	int numDims;
	double offsets[3];
	double spacings[3];
	string imgDims;
	vector<double> fixed;
	vector<double> moving;
};

// Function prototypes
LandmarkPairs readLandmarksIx(string, string, string);
LandmarkPairs readLandmarksIreg(string);
void writeLandmarksTransformix(LandmarkPairs, string, string);
void writeLandmarksSlicer(LandmarkPairs, string, string, bool);
void writeLandmarksText(LandmarkPairs, string, string, bool);

int main(int argc, char *argv[])
{
    
/*-----------------------------------------------------------------------------
//////////////////////////  Parse Input Arguments   ///////////////////////////
-----------------------------------------------------------------------------*/
    
    // Check is performed to assure that proper number of arguments were given.
    if(argc != 11)
    {
            cout << "\nUnexpected number of parameters!\n";
            // Correct usage of program is displayed to the user.
            cout << "Required arguments: -in_file <pathToInputLandmarks>";
			cout << " -in_type <inputLandmarksFormat>";
            cout << " -out_dir <pathToOutputDirectory>";
            cout << " -out_type <outputLandmarksFormat>";
			cout << " -keep_all <0 or 1>\n\n";
            return EXIT_FAILURE;
            
    }// end if
    
    // Variables to hold input arguments are declared.
    string pathInput, inputType, pathOutput, outputType, keep_all;
    
    // Arguments are parsed.
    for(int iArg = 1; iArg < argc; iArg =iArg+2)
    {
            
            // Path to input file is saved.
            if(string(argv[iArg]) == "-in_file")
            {
                       pathInput = argv[iArg+1];
            }
            // Format of input landmarks is saved.
            else if(string(argv[iArg])== "-in_type")
            {
                       inputType = argv[iArg+1];
            }
            // Path to output directory is saved.
            else if(string(argv[iArg])== "-out_dir")
            {
                       pathOutput = argv[iArg+1];
            }
            // Format of output files is saved.
            else if(string(argv[iArg])== "-out_type")
            {
                       outputType = argv[iArg+1];
            }
			// Whether to keep uncertain points is noted.
            else if(string(argv[iArg])== "-keep_all")
            {
				       keep_all = argv[iArg+1];
            }
            else //Otherwise, improper parameters were given.
            {
                cout << "\nUnexpected parameters!\n";
                // Correct usage of program is displayed to the user.
                cout << "Required arguments: -in_file <pathToInputLandmarks>";
                cout << " -in_type <inputLandmarksFormat>";
                cout << " -out_dir <pathToOutputDirectory>";
                cout << " -out_type <outputLandmarksFormat>";
				cout << " -keep_all <0 or 1>\n\n";
                return EXIT_FAILURE;
            } // end if/elseif/else
    
    } // end for
	
	// I/O file types are checked for compatibility.
	if ((inputType.compare("ireg") == 0)&&(outputType.compare("tfx_lmk") == 0))
	{
		cout << "Landmark list to Transformix parameters is not supported.\n";
		return EXIT_FAILURE;
	}
    
/*-----------------------------------------------------------------------------
//////////////////////////   Read Input Landmarks   ///////////////////////////
-----------------------------------------------------------------------------*/
    
    // Conversion process is started.
    cout << "\nStarting conversion...";
	
	LandmarkPairs readPair;
    
    // The conversion function matching the input landmarks format is called.
    if (inputType == "ix_pp") // iX point pairs specified.
    {
        readPair= readLandmarksIx(pathInput, outputType, keep_all);
    }
    else if (inputType == "ireg") // Caliper registration output specified.
    {
        readPair = readLandmarksIreg(pathInput);
    }
    else // Incorrect format was specified.
    {
        cout << "\nUnexpected input format!\n";
        cout << "Options are: ix_pp, ireg\n";
        return EXIT_FAILURE;     
    }
	
	if (&readPair == NULL)
	{
		cout << "Unsuccessful read";
		return EXIT_FAILURE;
	}
	
/*-----------------------------------------------------------------------------
/////////////////////////   Write Output Landmarks   //////////////////////////
-----------------------------------------------------------------------------*/

    // Write process is started.
    cout << "Starting write...\n";
    
    // The write function matching the output landmarks format is called.
    if (outputType == "tfx_lmk") // Transformix parameter file.
    {
        writeLandmarksTransformix(readPair, pathInput, pathOutput);
    }
    else if (outputType == "slr_fid") // Slicer fiducials.
    {
        writeLandmarksSlicer(readPair, pathInput, pathOutput, true);
		
		if (inputType == "ix_pp")
		{
		    writeLandmarksSlicer(readPair, pathInput, pathOutput, false);
		}
    }
	else if (outputType == "std_txt") // Plain text.
    {
        writeLandmarksText(readPair, pathInput, pathOutput, true);
		
		if (inputType == "ix_pp")
		{
		    writeLandmarksText(readPair, pathInput, pathOutput, false);
		}
    }
    else // Incorrect format was specified.
    {
        cout << "\nUnexpected output format!\n";
        cout << "Options are: tfx_lmk, slr_fid\n";
        return EXIT_FAILURE;     
    }
	
	cout << "Conversion complete!\n\n";
	
    return EXIT_SUCCESS;
    
} // end main



//***********************************************************
// Function readLandmarksIx is defined.                     *
// The function reads landmark coordinates from an iX point *
// pairs file and converts them into physical coordinates   *
// before returning them in a vector.                       *
//***********************************************************

LandmarkPairs readLandmarksIx(string pathInput, string outType, string keep_all)
{

    // Output landmark pairs structure is created.
	LandmarkPairs pairs;
                     
    // I/O files and string to hold read lines are declared.
    ifstream pointPairs;
	ifstream fixedMhd;
    string currentLine;
    
    // Variable holding the number of dimensions is declared and initialized.
    const int NUM_DIMS = 3;
	
	// Vectors created to hold converted coordinates.
	vector<double> fixedCoordsVector;
	vector<double> movingCoordsVector;
    
    /*--------------------------------------------------------------------------
    ////////////////////////  Open point pairs files  //////////////////////////
    --------------------------------------------------------------------------*/
    
    // Point pairs file is opened.
    cout << "\nOpening point pairs file: ";
    cout << pathInput << endl;
    pointPairs.open(pathInput.c_str());
    
    // Check is performed for successful file open.
    if (!(pointPairs.is_open()))
    {
         cout << "Failed to open point pairs file!\n\n";
    }
    else
    {
        cout << "Successfully opened point pairs file.\n";
    }
    
    /*--------------------------------------------------------------------------
    //////////////////////////  Read point pairs file  /////////////////////////
    --------------------------------------------------------------------------*/
	
	//Declares strings to hold paths to fixed and moving images
	string pathMhdFixed;
	string pathMhdMoving;
    
    //Declares and initializes strings
    //of point pair file parameters
    string strPoint = "Point_X->";
    string strPointDim = "Point_0->0";
    string strManChosen = "ManuallyChosen=";
    string strSysGuess = "_SystemGuess=";
    string strUnsure ="VeryUnsure=";
    string strPointCorr = "Point_0->0_Corresp=";

    //Keeps track of current line
    int lineNum = 0;
    
    //Reads file path lines of iX output and
    //positions ifstream to begin reading points
    pointPairs >> currentLine;
	pathMhdFixed = currentLine;
	
	//Checks if Windows file path is present and replaces it with Linux
	//compatible file path. Removes leading "Scan_x=" characters, too.
	if(pathMhdFixed[7]=='Z')
	{
	    pathMhdFixed.erase(0,9);
		pathMhdFixed.insert(0,"/rdo/home/cguy");
		for(int i = 0; i < pathMhdFixed.length(); i++)
		{
		    if(pathMhdFixed[i] == '\\')
			    pathMhdFixed.replace(i,1,"/");
		}
	}
	else if (pathMhdFixed[7] =='X')
	{
	    pathMhdFixed.erase(0,9);
		pathMhdFixed.insert(0,"/rdo/home/cguy/ix");
		for(int i = 0; i < pathMhdFixed.length(); i++)
		{
		    if(pathMhdFixed[i] == '\\')
			    pathMhdFixed.replace(i,1,"/");
		}
	}
	else
	{
	    pathMhdFixed.erase(0,7);
	}
	
    lineNum++;
    pointPairs >> currentLine;
	pathMhdMoving = currentLine;
    lineNum++;
    
    //Declares SystemGuess and numPointsFound flag
    bool systemGuessPresent;
    bool numPointsFound = false;
    
    //While points remaining
    while(!(pointPairs.eof()))
    {
        //Initializes SystemGuess flag
        systemGuessPresent = false;
                              
        //Reads Distinctiveness
        pointPairs >> currentLine;
        lineNum++;
        
        //Corrects point numbering format to match current file
        if(!numPointsFound)
        {
             //If three digits per point number
             if(currentLine[9] == '-')
             {                 
                 strPoint.insert(6,"XX");
                 strPointDim.insert(6,"00");
                 strPointCorr.insert(6,"00");
             }
             //If two digits per point number
             else if(currentLine[8] == '-')
             {
                 strPoint.insert(6,"X");
                 strPointDim.insert(6,"0");
                 strPointCorr.insert(6,"0");
             }
             //Else one digit per point number
             numPointsFound = true;    
        }//end if numPointsFound
        
        //Makes sure end of file not reached
        if(!pointPairs.fail())
        {
            //Reads ManuallyChosen
            pointPairs >> currentLine;
            lineNum++;
        
            //Verifies ManuallyChosen is present then extracts value
            if(currentLine.compare(strPoint.length(),
                  strManChosen.length(), strManChosen) == 0)
            {
                currentLine = currentLine.erase(0,(strPoint.length()
                                                  + strManChosen.length()));
            }
            else // When file structure incorrect
            {
                cout << "Error reading point pair file value: ManuallyChosen";
                cout << " at line " << lineNum << endl;
                cout << currentLine;
            }
        
            //If point was not manually chosen, point
            //coordinates will appear sequentially
            if(currentLine.compare("0") == 0)
            {
               //Reads SqDiffRegion and VeryUnsure
               //NOTE: If automatically chosen,
               //      VeryUnsure will always be false
               pointPairs >> currentLine;
               lineNum++;
               pointPairs >> currentLine;
               lineNum++;
           
               //Extracts X,Y,Z coordinates of point pair
               for(int j = 0; j < NUM_DIMS; j++)
               {
                   
                    //Gets coodinate line
                    pointPairs >> currentLine;
                    lineNum++;
              
                    //Checks for SystemGuess and skips if present
                    if(currentLine.compare(strPointDim.length(),
                          strSysGuess.length(),strSysGuess)==0)
                    {
                        pointPairs >> currentLine;
                        systemGuessPresent = true;
                    }
              
                    //Gets fixed coordinate
                    //Erases "Point_000->0=" from start of line
                    currentLine == currentLine.erase(0,
                                                   (strPointDim.length() + 1));
                    fixedCoordsVector.push_back(atoi(currentLine.c_str()));
           
                    //Gets moving coodinate
                    pointPairs >> currentLine;
                    lineNum++;
                    //Erases "Point_000->0_Corresp=" form start of line
                    currentLine == currentLine.erase(0,strPointCorr.length());
                    movingCoordsVector.push_back(atoi(currentLine.c_str()));
              
                 }//end for
            }
            else //If point was manually chosen
            {
             
                 //Reads SqDiffRegion and VeryUnsure
                 pointPairs >> currentLine;
                 lineNum++;
                 pointPairs >> currentLine;
                 lineNum++;           

                 //Verifies VeryUnsure is present then extracts value
                 if((currentLine.compare(strPoint.length(),
                       strUnsure.length(), strUnsure) == 0))
                 {
                     currentLine = currentLine.erase(0,(strPoint.length()
                                                   +  strUnsure.length()));
                 }
                 else // When file structure incorrect
                 {
                     cout << "\nError reading point pair file value:";
                     cout << " VeryUnsure at line " << lineNum << endl;
                 }
        
                 //Checks if chosen pair is very uncertain
                 if((currentLine.compare("0")) && !keep_all.compare("0"))
                 {                       
                     for (int jj = 0; jj < (2 * NUM_DIMS); jj++)
                     {
                         pointPairs >> currentLine;
                         lineNum++;
                    
                         //Checks for SystemGuess and skips if present
                         if(currentLine.compare(strPointDim.length(),
                                          strSysGuess.length(),strSysGuess)==0)
                         {
                             pointPairs >> currentLine;
                             lineNum++;
                             systemGuessPresent = 1;
                         }
                     }//end for      
                 }
                 else //Keeps the pair
                 {
					 //Extracts X,Y,Z coordinates of point pair
                     for(int j = 0; j < NUM_DIMS; j++)
                     {
                         //Gets coodinate line
                         pointPairs >> currentLine;
                         lineNum++;
                   
                         //Checks for SystemGuess and skips if present
                         if(currentLine.compare(strPointDim.length(),
                              strSysGuess.length(), strSysGuess)==0)
                         {
                             pointPairs >> currentLine;
                             lineNum++;
                             systemGuessPresent = 1;
                         }
                   
                         //Gets fixed coordinate
                         //Erases "Point_000->0=" from start of line
                         currentLine == currentLine.erase(0,
                                     (strPointDim.length() + 1));
                         fixedCoordsVector.push_back(atoi(currentLine.c_str()));
           
                         //Gets moving coodinate
                         pointPairs >> currentLine;
                         lineNum++;
                         //Erases "Point_000->0_Corresp=" form start of line
                         currentLine == currentLine.erase(0,
						                                strPointCorr.length());
                         movingCoordsVector.push_back(atoi(currentLine.c_str()));
              
                    }//end for 
                
                 }//end VeryUnsure if/else
           
              } //end ManuallyChosen if/else
        
              //Skips last SystemGuess value if present
              if(systemGuessPresent)
              {
                  pointPairs >> currentLine;
                  lineNum++;
              } 
        }//end if blank line

    }//end while
	
	/*-------------------------------------------------------------------------
    ////////////////////////  Open point pairs files  /////////////////////////
    -------------------------------------------------------------------------*/
	
	//Opens MetaHeader file
    cout << "Opening MetaHeader file: ";
    cout << pathMhdFixed << endl;
    fixedMhd.open(pathMhdFixed.c_str());
    
    //Checks for successful file open
    if (!(fixedMhd.is_open()))
    {
        cout << "Failed to open fixed image file!\n";
    }
    else
    {
        cout << "Successfully opened fixed image file.\n";
    }
    
    
    /*-------------------------------------------------------------------------
    /////////////////////////  Read Meta Header file  /////////////////////////
    -------------------------------------------------------------------------*/
    
    //Declares variables to hold data which
    //will be extracted from fixed image file
    string orientation; //Holds orienation matrix
    string imgDim;      //Holds image dimensions
    string offset;      //Holds image offset
    string spacing;     //Holds element spacing
    
    //Declares and initializes strings which
    //begin corresponding lines for extraction
    string strOrientation = "Orientation = ";
    string strImgDim = "DimSize = ";
    string strOffset = "Offset = ";
    string strSpacing = "ElementSpacing = ";
    
    //Reads first line
    getline(fixedMhd, currentLine);
    
    //Continues reading file until end, storing necessary values
    while(!fixedMhd.eof())
    {
        //Checks for orientation tag and extracts data if present             
        if(currentLine.compare(0, strOrientation.length(), strOrientation)==0)
        {
             orientation = currentLine.erase(0, strOrientation.length());
        }
        //Checks for image dimensions tag and extracts data if present   
        else if(currentLine.compare(0, strImgDim.length(), strImgDim)==0)
        {
             imgDim = currentLine.erase(0, strImgDim.length());
        }
        //Checks for offset tag and extracts data if present   
        else if(currentLine.compare(0, strOffset.length(), strOffset)==0)
        {
             offset = currentLine.erase(0, strOffset.length());
        }
        //Checks for element spacing tag and extracts data if present   
        else if(currentLine.compare(0, strSpacing.length(), strSpacing)==0)
        {
             spacing = currentLine.erase(0, strSpacing.length());
        }
        
        //Reads next line
        getline(fixedMhd, currentLine);
        
    }//end while
    

    /*-------------------------------------------------------------------------
    ///////////////////////  Extract offset components  ///////////////////////
    -------------------------------------------------------------------------*/
    
    //Initializes variables
    //for data extraction
    float offsets [3] = {0,0,0};  //Holds component offsets
    std::istringstream inputOffset(offset);
    inputOffset >> offsets[0] >> offsets[1] >> offsets[2];
	                            
                                    
    /*-------------------------------------------------------------------------
    //////////////////////  Extract spacing components  ///////////////////////
    -------------------------------------------------------------------------*/
    
    float spacings [3] = {0,0,0};  //Holds component spacings
    std::istringstream inputSpacing(spacing);
    inputSpacing >> spacings[0] >> spacings[1] >> spacings[2];
									  
									  
	/*-------------------------------------------------------------------------
    ////////////////////  Convert to Physical Coordinates  ////////////////////
    -------------------------------------------------------------------------*/								  
									  
	// Variable to hold current landmark number is declared.
	int landmarkNum;
	
	// The structure variables are assigned.
	pairs.numPoints = fixedCoordsVector.size()/3;
	pairs.numDims = NUM_DIMS;
	pairs.imgDims = imgDim;
	
	// The offsets are saved.
	pairs.offsets[0] = offsets[0];
	pairs.offsets[1] = offsets[1];
	pairs.offsets[2] = offsets[2];
	
	//The spacing are saved.
	pairs.spacings[0] = spacings[0];
	pairs.spacings[1] = spacings[1];
	pairs.spacings[2] = spacings[2];
	
	// Voxel coordinates are converted to physical coordinates as they are 
	// stored in their respective arrays.
	for (int iCoord = fixedCoordsVector.size(); iCoord > 0 ; iCoord = iCoord - 3)
	{
		
		

    // The physical coordinates are calculated and stored.
	pairs.fixed.push_back(((fixedCoordsVector.at(iCoord-3)) *
                             spacings[0]) + offsets[0]);
    pairs.fixed.push_back(((fixedCoordsVector.at(iCoord-2)) *
		                     spacings[1]) + offsets[1]);
    pairs.fixed.push_back(((fixedCoordsVector.at(iCoord-1)) *
		                     spacings[2]) + offsets[2]);
		
    // The moving coordinates are calculated and stored.
	pairs.moving.push_back(((movingCoordsVector.at(iCoord-3)) *
		                     spacings[0]) + offsets[0]);
    pairs.moving.push_back(((movingCoordsVector.at(iCoord-2)) *
		                     spacings[1]) + offsets[1]);
	pairs.moving.push_back(((movingCoordsVector.at(iCoord-1)) *
		                     spacings[2]) + offsets[2]);
	
	} // end for iCoord
	
	// Files which were opened are closed.
	pointPairs.close();
	fixedMhd.close();
	
	// Order of landmarks is reversed, putting them back into original order.
	reverse(pairs.fixed.begin(), pairs.fixed.end());
	reverse(pairs.moving.begin(), pairs.moving.end());
	
    return pairs;
     
} // end readLandmarksIx



//**************************************************************
// Function readLandmarksIreg is defined.                      *
// The function reads landmark coordinates from an ireg result *
// landmark file before returning them in a vector.            *
//**************************************************************

LandmarkPairs readLandmarksIreg(string pathInput)
{
	LandmarkPairs pairs;
	
	// I/O file and string to hold read lines are declared.
    ifstream landmarkCoords;
	string currentLine;
	
	// Variable holding the number of dimensions is declared and initialized.
    const int NUM_DIMS = 3;
	
	// Vector created to hold coordinates.
	vector<double> coordsVector;
	
	/*--------------------------------------------------------------------------
    /////////////////////////  Open landmarks file  ////////////////////////////
    --------------------------------------------------------------------------*/
    
    // Point pairs file is opened.
    cout << "\nOpening landmarks file: ";
    cout << pathInput << endl;
    landmarkCoords.open(pathInput.c_str());
    
    // Check is performed for successful file open.
    if (!(landmarkCoords.is_open()))
    {
         cout << "Failed to open point pairs file!\n";
    }
    else
    {
        cout << "Successfully opened point pairs file.\n";
    }
	
	
	/*--------------------------------------------------------------------------
    ///////////////////////////  Read landmarks file  //////////////////////////
    --------------------------------------------------------------------------*/
	
	// All coordinates are extracted from file.
	while(!landmarkCoords.eof())
	{
	    landmarkCoords >> currentLine;
	    coordsVector.push_back(atof(currentLine.c_str()));
	}
	
	// Removes duplicate final value.
	coordsVector.pop_back();
	
	// Erase first value if number of values was not found to be divisible by 3,
	// in which case first value is number of points rather than a coordinate.
	if ((coordsVector.size() % 3) != 0)
	{
	    coordsVector.erase(coordsVector.begin());
	}
	
	// The physical coordinates are stored.
	for (int iCoord = 0; iCoord < coordsVector.size() ; iCoord++)
	{
	    pairs.fixed.push_back(coordsVector.at(iCoord));
	} // end for iCoord
	
	// Input file is closed.
	landmarkCoords.close();
	
	pairs.numDims = 3;
	pairs.numPoints = (coordsVector.size()/3);
	
    // Order of landmarks is reversed.
	reverse(pairs.fixed.begin(), pairs.fixed.end());
	
    return pairs;
     
} // end readLandmarksIreg


//**************************************************************
// Function writeLandmarksTransformix is defined.              *
// The function write landmarks into a parameter file for      *
// Transformix which can be used to perform a landmark-based   *
// transformation.                                             *
//**************************************************************

void writeLandmarksTransformix(LandmarkPairs pairs, string pathPointPairs, string outPath)
{

/*-----------------------------------------------------------------------------
///////////////////////////// Creates Output File /////////////////////////////
-----------------------------------------------------------------------------*/
    
    //Gets path for directory of pointpair file
    string dirName = pathPointPairs;
    
    //Signals position where filename begins is found
    bool fileNameFound = false;
    
    //Signals extension position found
    bool extensionFound = false;
    
    //Starting positions for directory and filename
    int startFileName = 0;
    int startParentDir = 0;
    int startExtension = 0;
    
    //Searches for positions where filename begins
    //and parent directory begins
    for(int k = dirName.length(); k >= 0; k--)
    {
        //Searches for and reconds position of second backslash (Windows)
		//or forward slash (Linux), which indicates start of parent
		//directory of point pairs file
        if(((fileNameFound)&&(dirName[k] == 0x5c))||
		                               ((fileNameFound)&&(dirName[k] == 0x2f)))
        {
            startParentDir = k+1;
        }
        //Searches for and records position of first backslash (Windows) or
		//forward slash (Linux), which indicates start of filename of point
		//pairs file
        else if(((!fileNameFound)&&(dirName[k] == 0x5c))||
		                              ((!fileNameFound)&&(dirName[k] == 0x2f)))
        {
            startFileName = k+1;
            fileNameFound= true;
        }
        //Searches for and records position of first period,
        //which indicates start of file extension of point pairs file
        if((!extensionFound)&&(dirName[k] == 0x2e))
        {
            startExtension = k;
            extensionFound = true;
        }
    }//end for
    
    //Extracts filename by removing file extension and filepath
    string fileName = dirName;
    fileName.erase(startExtension,(fileName.length() - startExtension));
    fileName.erase(0,startFileName);
    
    //Creates path to output file
    string outputFilePath = outPath;
    outputFilePath += fileName;
    outputFilePath += "_transformix.txt";
    
    //Creates and opens output file
    cout << "Creating output file: ";
	cout << outputFilePath << endl;
    ofstream outputFile(outputFilePath.c_str());
    
    //Checks for successful file open
    if (!(outputFile.is_open()))
    {
         cout << "Failed to create output file!\n";
    }

/*-----------------------------------------------------------------------------
///////////////////////////// Writes Output File //////////////////////////////
-----------------------------------------------------------------------------*/
    
    //Writes transform-specific information
    outputFile << "(Transform \"SplineKernelTransform\")\n";
    outputFile << "(NumberOfParameters " << (pairs.numDims * pairs.numPoints) << ")\n";
    outputFile << "(TransformParameters";
    
    //Writes moving coordinates to output file
	for (int i = 0; i < (pairs.numDims * pairs.numPoints); i = i + 3)
	{
	    outputFile << " ";
		outputFile << pairs.moving.at(i + 2);
		outputFile << " ";
		outputFile << pairs.moving.at(i + 1);
		outputFile << " ";
		outputFile << pairs.moving.at(i);
	}
    
    //Continues writing transform-specific information
    outputFile << ")\n";
    outputFile << "(InitialTransformParametersFileName";
    outputFile << " \"NoInitialTransform\")\n";
    outputFile << "(HowToCombineTransforms \"Compose\")\n\n";
    
    //Writes image-specific information
    outputFile << "// Image specific\n";
    outputFile << "(FixedImageDimension 3)\n";
    outputFile << "(MovingImageDimension 3)\n";
    outputFile << "(FixedInternalImagePixelType \"float\")\n";
    outputFile << "(MovingInternalImagePixelType \"float\")\n";
    outputFile << "(Size " << pairs.imgDims << ")\n";
    outputFile << "(Index 0 0 0)\n";
    outputFile << "(Spacing " << pairs.spacings[0] << " ";
    outputFile << pairs.spacings [1] << " " << pairs.spacings[2] << ")\n";
    outputFile << "(Origin " << pairs.offsets[0] << " " << pairs.offsets[1];
    outputFile << " " << pairs.offsets[2] << ")\n";
    outputFile << "(Direction 1.0000000000 0.0000000000 0.0000000000 ";
    outputFile <<            "0.0000000000 1.0000000000 0.0000000000 ";
    outputFile <<            "0.0000000000 0.0000000000 1.0000000000)\n";
    outputFile << "(UseDirectionCosines \"true\")\n\n";
    
    //Writes SplineKernelTransform-specific information
    outputFile << "// SplineKernelTransform specific\n";
    outputFile << "(SplineKernelType \"ThinPlateSpline\")\n";
    outputFile << "(SplinePoissonRatio 0.0)\n";
    outputFile << "(SplineRelaxationFactor 0.0)\n";
    outputFile << "(FixedImageLandmarks";
    
    //Writes fixed coordinates to output file
	for (int i = 0; i < (pairs.numDims * pairs.numPoints); i = i + 3)
	{
	    outputFile << " ";
		outputFile << pairs.fixed.at(i + 2);
		outputFile << " ";
		outputFile << pairs.fixed.at(i + 1);
		outputFile << " ";
		outputFile << pairs.fixed.at(i);
	}
	
    outputFile << ")\n\n";
    
    //Writes ResamplerInterpolator-specific information
    outputFile << "// ResampleInterpolator specific\n";
    outputFile << "(ResampleInterpolator \"FinalBSplineInterpolator\")\n";
    outputFile << "(FinalBSplineInterpolationOrder 3)\n\n";
    
    //Writes Resampler-specific information
    outputFile << "// Resampler specific\n";
    outputFile << "(Resampler \"DefaultResampler\")\n";
    outputFile << "(DefaultPixelValue 0.000000)\n";
    outputFile << "(ResultImageFormat \"mhd\")\n";
    outputFile << "(ResultImagePixelType \"short\")\n";
    outputFile << "(CompressResultImage \"false\")\n";
    
    //Closes files
    outputFile.close();

    return;
     
} // end writeLandmarksTransformix


//**************************************************************
// Function writeLandmarksSlicer is defined.                   *
// The function writes the landmarks to a fiducial file for 3D *
// Slicer which can be used to visualize the landmarks with    *
// respect to the patient anatomy.                             *
//**************************************************************

void writeLandmarksSlicer(LandmarkPairs pairs, string inPath, string outPath, bool writeFixed)
{

	/*-------------------------------------------------------------------------
    /////////////////////////// Creates Output File ///////////////////////////
    -------------------------------------------------------------------------*/
    
    //Gets path for directory of pointpair file
    string dirName = inPath;
    
    //Signals position where filename begins is found
    bool fileNameFound = false;
    
    //Signals extension position found
    bool extensionFound = false;
    
    //Starting positions for directory and filename
    int startFileName = 0;
    int startParentDir = 0;
    int startExtension = 0;
    
    //Searches for positions where filename begins
    //and parent directory begins
    for(int k = dirName.length(); k >= 0; k--)
    {
        //Searches for and reconds position of second backslash (Windows)
		//or forward slash (Linux), which indicates start of parent
		//directory of point pairs file
        if(((fileNameFound)&&(dirName[k] == 0x5c))||
		                               ((fileNameFound)&&(dirName[k] == 0x2f)))
        {
            startParentDir = k+1;
        }
        //Searches for and records position of first backslash (Windows) or
		//forward slash (Linux), which indicates start of filename of point
		//pairs file
        else if(((!fileNameFound)&&(dirName[k] == 0x5c))||
		                              ((!fileNameFound)&&(dirName[k] == 0x2f)))
        {
            startFileName = k+1;
            fileNameFound= true;
        }
        //Searches for and records position of first period,
        //which indicates start of file extension of point pairs file
        if((!extensionFound)&&(dirName[k] == 0x2e))
        {
            startExtension = k;
            extensionFound = true;
        }
    }//end for
    
    //Extracts filename by removing file extension and filepath
    string fileName = dirName;
    fileName.erase(startExtension,(fileName.length() - startExtension));
    fileName.erase(0,startFileName);
    
    //Creates path to output file
    string outputFilePath = outPath;
    outputFilePath += fileName;
	if(writeFixed)
	{
        outputFilePath += "_fixed_slicer.fcsv";
    }
	else
	{
	    outputFilePath += "_moving_slicer.fcsv";
	}
	
    //Creates and opens output file
    cout << "Creating output file: ";
	cout << outputFilePath << endl;
    ofstream outputFile(outputFilePath.c_str());
    
    //Checks for successful file open
    if (!(outputFile.is_open()))
    {
         cout << "Failed to create output file!\n";
    }
	
	/*-------------------------------------------------------------------------
    /////////////////////////// Writes Output File ////////////////////////////
    -------------------------------------------------------------------------*/
    
    //Writes fiducial set information
    outputFile << "# name = lmk\n";
    outputFile << "# numPoints = ";
    outputFile << pairs.numPoints;
	outputFile << "\n";
    outputFile << "# symbolScale = 5.5\n";
    outputFile << "# symbolType = 11\n";
    outputFile << "# visibility = 1\n";
    outputFile << "# textScale = 12.5\n";
    outputFile << "# color = 0.4,1,1\n";
    outputFile << "# selectedColor = 0.807843,0.560784,1\n";
    outputFile << "# opacity = 1\n";
    outputFile << "# ambient = 0\n";
    outputFile << "# diffuse = 1\n";
    outputFile << "# specular = 0\n";
    outputFile << "# power = 1\n";
    outputFile << "# locked = 1\n";
    outputFile << "# columns = label,x,y,z,sel,vis\n";
    
	// If fixed coordinates are to be written...
	if (writeFixed)
	{
        //Writes fixed coordinates to output file
	    for (int i = 0; i < (pairs.numDims * pairs.numPoints); i = i+3)
	    {
		    if (i != 0)
		    outputFile << "\n";
	        outputFile << ((i/pairs.numDims)+1);
		    outputFile << ", ";
		    outputFile << (pairs.fixed.at(i + 2) * -1.0);
		    outputFile << ", ";
		    outputFile << (pairs.fixed.at(i + 1) * -1.0);
		    outputFile << ", ";
		    outputFile << pairs.fixed.at(i);
		    outputFile << ", 0, 1";
	    }
	}
	else
	{
	    //Writes moving coordinates to output file
	    for (int i = 0; i < (pairs.numDims * pairs.numPoints); i = i+3)
	    {
		    if (i != 0)
		    outputFile << "\n";
	        outputFile << ((i/pairs.numDims)+1);
		    outputFile << ", ";
		    outputFile << (pairs.moving.at(i + 2) * -1.0);
		    outputFile << ", ";
		    outputFile << (pairs.moving.at(i + 1) * -1.0);
		    outputFile << ", ";
		    outputFile << pairs.moving.at(i);
		    outputFile << ", 0, 1";
	    }
	}
	
	// Closes output file.
	outputFile.close();

    return;
     
} // end writeLandmarksSlicer





//**************************************************************
// Function writeLandmarksSlicer is defined.                   *
// The function writes the landmarks to a fiducial file for 3D *
// Slicer which can be used to visualize the landmarks with    *
// respect to the patient anatomy.                             *
//**************************************************************

void writeLandmarksText(LandmarkPairs pairs, string inPath, string outPath, bool writeFixed)
{

	/*-------------------------------------------------------------------------
    /////////////////////////// Creates Output File ///////////////////////////
    -------------------------------------------------------------------------*/
    
    //Gets path for directory of pointpair file
    string dirName = inPath;
    
    //Signals position where filename begins is found
    bool fileNameFound = false;
    
    //Signals extension position found
    bool extensionFound = false;
    
    //Starting positions for directory and filename
    int startFileName = 0;
    int startParentDir = 0;
    int startExtension = 0;
    
    //Searches for positions where filename begins
    //and parent directory begins
    for(int k = dirName.length(); k >= 0; k--)
    {
        //Searches for and reconds position of second backslash (Windows)
		//or forward slash (Linux), which indicates start of parent
		//directory of point pairs file
        if(((fileNameFound)&&(dirName[k] == 0x5c))||
		                               ((fileNameFound)&&(dirName[k] == 0x2f)))
        {
            startParentDir = k+1;
        }
        //Searches for and records position of first backslash (Windows) or
		//forward slash (Linux), which indicates start of filename of point
		//pairs file
        else if(((!fileNameFound)&&(dirName[k] == 0x5c))||
		                              ((!fileNameFound)&&(dirName[k] == 0x2f)))
        {
            startFileName = k+1;
            fileNameFound= true;
        }
        //Searches for and records position of first period,
        //which indicates start of file extension of point pairs file
        if((!extensionFound)&&(dirName[k] == 0x2e))
        {
            startExtension = k;
            extensionFound = true;
        }
    }//end for
    
    //Extracts filename by removing file extension and filepath
    string fileName = dirName;
    fileName.erase(startExtension,(fileName.length() - startExtension));
    fileName.erase(0,startFileName);
    
    //Creates path to output file
    string outputFilePath = outPath;
    outputFilePath += fileName;
	if(writeFixed)
	{
        outputFilePath += "_fixed_landmarks.txt";
    }
	else
	{
	    outputFilePath += "_moving_landmarks.txt";
	}
    
    //Creates and opens output file
    cout << "Creating output file...\n";
    ofstream outputFile(outputFilePath.c_str());
    
    //Checks for successful file open
    if (!(outputFile.is_open()))
    {
         cout << "Failed to create output file!\n";
    }
	
	/*-------------------------------------------------------------------------
    /////////////////////////// Writes Output File ////////////////////////////
    -------------------------------------------------------------------------*/
    outputFile << "point\n";
	outputFile << pairs.numPoints;
	outputFile << "\n";
	
	// If fixed coordinates are to be written...
	if (writeFixed)
	{
        //Writes fixed coordinates to output file
	    for (int i = 0; i < (pairs.numDims * pairs.numPoints); i = i+3)
	    {
		    outputFile << pairs.fixed.at(i + 2);
		    outputFile << " ";
		    outputFile << pairs.fixed.at(i + 1);
		    outputFile << " ";
		    outputFile << pairs.fixed.at(i);
		    outputFile << "\n";
	    }
	}
	else
	{
	    //Writes moving coordinates to output file
	    for (int i = 0; i < (pairs.numDims * pairs.numPoints); i = i+3)
	    {
		    outputFile << pairs.moving.at(i + 2);
		    outputFile << " ";
		    outputFile << pairs.moving.at(i + 1);
		    outputFile << " ";
		    outputFile << pairs.moving.at(i);
		    outputFile << "\n";
	    }
	}
	
	// Closes output file.
	outputFile.close();

    return;
     
} // end writeLandmarksText

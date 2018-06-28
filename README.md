# landmarkQA
Code for performing landmark set quality assurance

To Use:
 * Compile LandmarkConverter.cpp with your local C++ compiler.
 * Run the converter using the desired input/output types (see below).

E.g. To convert from point pairs of isiMatch to a transformix landmark-based transformation file, discarding points marked as 'very unsure':
 LandmarkConverter -in_file <path to input .dat point pairs> -in_type ix_pp -out_dir . -out_type tfx_lmk -keep_all 0

Purpose:
This tool reformats landmark pairs from one input format (i.e. iX's Matching Points Annotator output) to another (i.e. transformix's landmark- based transform parameter file).
 
 The code is called along with five parameters:
 *  -in_file  The input file containing the landmarks
 *  -in_type  The type of input file from which landmarks will be read:
               ix_pp - Point pair file of landmarks matched with Image eXplorer (isiMatch)
                ireg - Registration landmarks from Caliper registration code.
 *  -out_dir  The path of the directory where the output file will be written
 *  -out_type The type of output file to be generated, options include:
               tfx_lmk  - Transformix landmark-based transform input file
               slr_fid  - 3D Slicer fiducial file
               std_txt  - Standard plain text file
 *  -keep_all For input of type 'ix_pp, whether to keep (1) or discard (0) points marked as 'very unsure'
 
The files are then read, storing necessary data to be included in the output file for Transformix. The output file is then written to the irectory specified.
 
 

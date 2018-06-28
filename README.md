# landmarkQA
Code for performing landmark set quality assurance

To Use:
 *  Compile LandmarkConverter.cpp with your local C++ compiler. 

Purpose:
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
 
 

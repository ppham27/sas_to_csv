sas_to_csv
==========

Converts SAS7BDAT files to a csv file. This is a port of the work done by: 
Matthew S. Shotwell, PhD 
Assistant Professor 
Department of Biostatistics 
Vanderbilt University 
matt.shotwell@vanderbilt.edu
See http://cran.r-project.org/web/packages/sas7bdat/vignettes/sas7bdat.pdf for details of the format. 


The main advantages to my program is speed and the fact that the parsing is done page by page, so that the computer will not run out of memory on large datasets.

Examples:

sas_to_csv iris.sas7bdat

sas_to_csv pizzazz.sas7bdat
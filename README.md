# PUK_Turbulence_2025_Code
Code used to code my project outside the course scope to analyse ERA5 gribdata with regards to 2D Kolmogorov scaling in atmopsheric turbulence

Here is a short list of the different files and what they do. The python pre-processing is not very optimized yet, so if anyone has a faster idea, feel free to comment on it. 

Pipeline.sh:
-----------
The overall bash to execute the analysis. First installs needed packages (special to the cluster I was working on), then downloads and prepares the data (Download_and_Process_Data.py), posts them as slurm jobs on the cluster (job.sh) executing Calc_Energy.cpp and then analyses and plots the results (Plotting.ipynb). This process is repeated for every YEAR in [1940,2024].

Download_and_Process_Data.py: 
----------------------------
Uses the cdsapi provided by the Copernicus Climate Data Store to download the grib data from the MARS servers of the ECMWF and to seperate the data into the different pressure levels, as defined in the array levels = [...] using pygrib.

Calc_Energy.cp:
--------------
Calculates the spectral energy distribution $E_n$ and the energy dependent on the radial wavenumber $k=\sqrt{n^2+m^2}$, $E(k(n,m))$ in parallel for all pressure levels. 

job.sh:
------
The slurm job bash script

Plotting.ipynb:
-------------
Plots and averages the energies over the different pressure levels for all previously computed years. 

# To be noted: 

The code uses the cdsapi request layout as by spring 2025. There could have happend possible changes in the mean time, so please adapt the code accordingly, if someone was to ever work with it again. To use the API you need to additionally store your login credentials in an .cdsapirc file in your root directory. Further information can be found here: 
https://cds.climate.copernicus.eu/how-to-api 

The parallaziation is done on the MODI cluster provided by the University of Copenhagen. The job.sh bash file needs to be adapted according to your cluster layout. 

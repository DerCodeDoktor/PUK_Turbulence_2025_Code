#!/usr/bin/env bash


pip3 install pygrib
pip3 install cdsapi
pip3 install pandas 

# creates credentials file in root folder 
#cp ./cdsapirc.txt $HOME/.cdsapirc

for YEAR in {1940..2024} 
do
    python3 Download_and_Process_Data.py $YEAR
    rm ./Downloaded_Data/output.grib
    mkdir ./logs 
    mkdir -p ./Results_Data/$YEAR/Kolmogorov
    sbatch -W  --output=./logs/log_$YEAR.out job.sh $YEAR
    rm -r ./Downloaded_Data 
done

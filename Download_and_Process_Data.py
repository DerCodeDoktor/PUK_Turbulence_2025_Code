import pygrib
import pandas as pd
import os
import cdsapi
import numpy as np 
import sys 



T = 639

annual_avg_arr = np.zeros([37,410240])
levels = [1,2,3,5,7,10,20,30,50,70,100,125,150,175,200,225,250,300,350,400,450,500,550,600,650,700,750,775,800,825,850,875,900,925,950,975,1000]

def WritetoCSV(grbs,m,foldername):
    
    for s in range(len(grbs.select()[37*m:37*(m+1)])):
        
        data = grbs.select()[37*m+s].values.reshape(int(grbs.select()[37*m+s].values.shape[0]/2),2) # reshape data to be [Re,Im]
        level = str(grbs.message(37*m+s+1)).split("level")[1].split(":")[0]

        annual_avg_arr[s,::] += grbs.select()[37*m+s].values
        
        filename = foldername + "/Data_level_" + level[1:]
        
        np.savetxt(filename+'.csv',data,delimiter=',')
        #print("Month {}: Level {} written".format(m+1,level))
        
    return 


year = sys.argv[1]
print(year)
months = 12
path = "./Downloaded_Data/{}".format(year)
os.makedirs(path, exist_ok=True)

request_string = ""
# constrcut string for api request:
for i in range(1,10):
    request_string += "{}0{}01/".format(year,i)

for i in range(10,13):
    request_string += "{}{}01/".format(year,i)


c = cdsapi.Client()
c.retrieve("reanalysis-era5-complete", {
    "class": "ea",
    "date": request_string[:-1],
    "expver": "1",
    "levelist": "1/2/3/5/7/10/20/30/50/70/100/125/150/175/200/225/250/300/350/400/450/500/550/600/650/700/750/775/800/825/850/875/900/925/950/975/1000",
    "levtype": "pl",
    "param": "138",
    "stream": "moda",
    "type": "an"
}, "./Downloaded_Data/output.grib")

grbs = pygrib.open('./Downloaded_Data/output.grib')

# write from grib to different CSVs for further analysis
for m in range(months):
    print("month {}".format(m+1))
    folderpath = path + "/{}".format(m+1)
    os.makedirs(folderpath, exist_ok=True)
    WritetoCSV(grbs,m,folderpath)

average_path = path+"/annual_average_Data/"

os.makedirs(average_path,exist_ok=True)
# write annual average data for different pressure levels
for r in range(annual_avg_arr.shape[0]):
    row = annual_avg_arr[r].reshape(int(annual_avg_arr[r].shape[0]/2),2)
    np.savetxt(average_path+"/ann_avg_level_{}.csv".format(levels[r]),1/months*row,delimiter=",")
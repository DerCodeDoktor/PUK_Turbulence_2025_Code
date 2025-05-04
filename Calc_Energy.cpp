#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <complex>
#include <string>
#include <chrono>
#include <format>
#include <array>
#include <mpi.h>

// Get the number of processes
int mpi_size;

// Get the rank of the process
int mpi_rank;

// Truncuation number for sprectral coefficients
const int T = 639;
const double a = 6371000.009; 

class ComplexCSVReader {
public:
    ComplexCSVReader(const std::string& filename) : filename(filename) {}

    std::vector<std::complex<double>> readCSV() {
        std::vector<std::complex<double>> complexNumbers;
        std::ifstream file(filename);

        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return complexNumbers;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string realPart, imagPart;

            // Read the real part from the first column and the imaginary part from the second column
            if (std::getline(ss, realPart, ',') && std::getline(ss, imagPart, ',')) {
                double real = std::stod(realPart);
                double imag = std::stod(imagPart);
                complexNumbers.emplace_back(real, imag);
            }
        }

        file.close();
        return complexNumbers;
    }

private:
    std::string filename;
};

void writeVectorToCSV(const std::vector<std::vector<std::complex<double>>>& V, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile) {
        std::cerr << "Error opening file for writing\n";
        return;
    }

    for (const auto& row : V) 
    {
        std::ostringstream oss;
        
        for (const auto& c : row) 
        {
            oss << c.real() << ",";
        }
        std::string line = oss.str();
        line.pop_back(); // Remove the trailing comma
        outFile << line << "\n";
    }

    outFile.close();
}

std::vector<std::complex<double>> calc_fluc_term(const int& level, const int& year)
{
    std::vector<std::complex<double>> fluc_term(205210,0.0); 
    
    for(int m=1; m <=12;m++)
    {
        std::string month_filename = std::format("./Downloaded_Data/{}/{}/Data_level_{}.csv",year,m,level);
        ComplexCSVReader month_reader(month_filename);

        std::vector<std::complex<double>> monthly_data = month_reader.readCSV();

        for(size_t j=0; j < monthly_data.size(); j++)
        {
           fluc_term[j] += (1.0/12.0)*(monthly_data[j]*std::conj(monthly_data[j]));     
        }

    }

    return fluc_term;
    
}

void write_Kolmogorov(const std::vector<std::complex<double>>& ann_array, const std::vector<std::complex<double>>& fluc_array, const int& level, const int& year){

    std::ofstream outFile_Kolmogorov(std::format("./Results_Data/{}/Kolmogorov/Kolmogorov_level_{}.csv",year,level));

    outFile_Kolmogorov << "k,E(k) \n";
    for(int m=0; m<=639;++m)
    {
        
        for(int n=m;n<=639;++n)
        {
           double k = sqrt(m*m+n*n);
           std::complex E = a*a/(n * (n + 1))*(ann_array[n+m]*std::conj(ann_array[n+m])+fluc_array[n+m]);
           if(m == 0)
           {
            E = E*1.0/4.0;
           }
           else
           {
            E = E*1.0/2.0;
           }
           outFile_Kolmogorov << k << "," << E.real() << "\n";
        }

    }
    outFile_Kolmogorov.close();
}


std::vector<std::complex<double>> calc_sum(/*const std::vector<std::complex<double>>& lv_mon_array*/const int level,const std::vector<std::complex<double>>& ann_array, const int& year) {

    std::vector<std::complex<double>> S;
    std::vector<std::complex<double>> fluc_array = calc_fluc_term(level,year);

     write_Kolmogorov(ann_array,fluc_array, level, year);
	

    for (int m = 0; m <= T; ++m) {
        std::vector<std::complex<double>> E_n_month_entries;
        std::vector<std::complex<double>> E_n_ann_entries;
        
        E_n_ann_entries.push_back(ann_array[m]);
        E_n_month_entries.push_back(fluc_array[m]);
        

        for (int i = 0; i < m; ++i) {
            
            // Find the index of the last element in E_n_entries
            // should be the same for both arrays, if I didnt fuck up the array sizes
            
            auto it = std::find(ann_array.begin(), ann_array.end(), E_n_month_entries.back());
            
            if (it != ann_array.end()) {
                int index = std::distance(ann_array.begin(), it);
                
                E_n_month_entries.push_back(fluc_array[index + T - i]);
                E_n_ann_entries.push_back(ann_array[index + T - i]);
            }
        }

        std::complex<double> sum = 0.0;

        // change equation to fit..thats left to do 

        for (size_t j = 0; j < E_n_ann_entries.size(); ++j) {
            sum += E_n_ann_entries[j] * std::conj(E_n_ann_entries[j])+E_n_month_entries[j];
        }

        S.push_back((2.0 * m * (m + 1)) / a * sum);
    }

    return S;
}

int main(int argc, char **argv) {

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    
    if(mpi_size >= 39)
    {
        throw std::invalid_argument("Number of ranks cant exceed number of pressure levels (37)\n");
        return 0; 
    }

    const int year = std::stoi(argv[1]);
    
    // define pressure levels 
    std::vector<int> levels = {1,2,3,5,7,10,20,30,50,70,100,125,150,175,200,225,250,300,350,400,450,500,550,600,650,700,750,775,800,825,850,875,900,925,950,975,1000};

    if(mpi_rank != mpi_size-1)
    {
     //auto begin = std::chrono::steady_clock::now();
     std::cout << "Reading data for pressure level " << levels[mpi_rank] << " on rank " << mpi_rank << "\n"; 

     //std::string monthly_filename = std::format("./home/bpt429_alumni_ku_dk/erda_mount/Blok 3/Puk/Downloaded_Data/1940/1/Data_level_{}.csv",levels[mpi_rank]);
        
     std::string annual_filename = std::format("./Downloaded_Data/{}/annual_average_Data/ann_avg_level_{}.csv",year,levels[mpi_rank]); 
     
    
     //ComplexCSVReader month_reader(monthly_filename);
     //std::vector<std::complex<double>> monthly_data = month_reader.readCSV();

     ComplexCSVReader annual_reader(annual_filename);
     std::vector<std::complex<double>> annual_data = annual_reader.readCSV();

     auto result = calc_sum(levels[mpi_rank],annual_data,year);
        
     MPI_Send(&result[0],T,MPI_C_DOUBLE_COMPLEX,mpi_size-1,mpi_rank,MPI_COMM_WORLD);
        
    }
    
    else if(mpi_rank == mpi_size-1)
    {
    auto begin = std::chrono::steady_clock::now();
    MPI_Request req[mpi_size-2];
    std::vector<std::vector<std::complex<double>>> Recv_buffer(mpi_size-1,std::vector<std::complex<double>>(T));
        
    for(int i=0;i<=mpi_size-2;i++)
    {
    MPI_Irecv(Recv_buffer[i].data(),T,MPI_C_DOUBLE_COMPLEX,i,i,MPI_COMM_WORLD,&req[i]);
    }
        
    MPI_Waitall(mpi_size-2,req,MPI_STATUSES_IGNORE);
    auto end = std::chrono::steady_clock::now();
	std::cout << "elapsed time at the end of gathering on rank "<< mpi_rank <<" : " << (end - begin).count() / 1000000000.0 << " sec" << std::endl;
        
    writeVectorToCSV(Recv_buffer,std::format("./Results_Data/{}/spectral_res_pressure_levels_{}.csv",year,year));

    std::cout << "Excuse me" << std::endl;
        
    }
    

    MPI_Finalize();

    return 0;    

}
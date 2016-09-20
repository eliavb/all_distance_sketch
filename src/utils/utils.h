#ifndef ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_UTILS_UTILS_H_
#define ALL_DISTANCE_SKETCH_ALL_DISTANCE_SKETCH_UTILS_UTILS_H_

#define FILE_SEPERATOR "/"
#define __DEBUG 0


namespace all_distance_sketch {
namespace utils{

typedef std::vector<int> SingleCommunity;
typedef std::vector< SingleCommunity > Communities;


class FileUtils {

public:

    typedef std::list< std::tuple< int, int, double> > NodePairList;

    static bool hasEnding (std::string const &fullString, std::string const &ending)
    {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
        } else {
            return false;
        }
    }

    static void getCommunityFromLine(std::string line, SingleCommunity * aComm, std::string delimiter){
        int i = 0;
        char * pch;
        char * p = new char[line.size() + 1];
        strcpy(p, line.c_str());
        pch = strtok(p, delimiter.c_str());
        while (pch != NULL) {
            int member = std::atoi(pch);
            pch = strtok(NULL, delimiter.c_str());
            ++i;
            aComm->push_back(member);
        }
        delete [] p;
    }

    static void getNodeIdsFromLine(std::string line, int * u, int * v, double* weight, std::string delimiter){
        double values[3] = {0, 0, 1};
        int i = 0;
        char * pch;
        char p[30];
        strcpy(p, line.c_str());
        pch = strtok(p, delimiter.c_str());
        while (pch != NULL) {
            values[i] = std::atoi(pch);
            pch = strtok(NULL, delimiter.c_str());
            ++i;
        }
        *u = values[0];
        *v = values[1];
        *weight = values[2];
        if (__DEBUG == 1){
            std::cout << "Node src=" << *u << " Node dest=" << *v << " weight=" << *weight << std::endl;
        }
    }

    static void GetCommunityFromFile(std::string aFilePath, Communities * aCommunities, std::string delimiter){
        std::string line;
        std::ifstream graphFile (aFilePath.c_str());
        if (__DEBUG){
            std::cout << "Extacting values from file=" << aFilePath << std::endl;
        }
        if (graphFile.is_open())
        {
            while ( getline (graphFile,line) )
            {
                SingleCommunity com;
                getCommunityFromLine(line, &com, delimiter);
                aCommunities->push_back(com);
            }
            graphFile.close();
        }
        else {
            std::cout << "Unable to open file" << std::endl; 
        }
    }

    static void GetNodePairListFromFile(std::string aFilePath, NodePairList * aNodePairList, std::string delimiter){
        std::string line;
        std::ifstream graphFile (aFilePath.c_str());
        int u, v;
        double w;
        if (__DEBUG){
            std::cout << "Extacting values from file=" << aFilePath << std::endl;
        }
        if (graphFile.is_open())
        {
            while ( getline (graphFile,line) )
            {
                getNodeIdsFromLine(line, &u, &v, &w, delimiter);
                std::tuple<int, int, double> nodePair = std::tuple<int, int, double>(u, v, w);
                aNodePairList->push_back(nodePair);
            }
            graphFile.close();
        }
        else {
            std::cout << "Unable to open file" << std::endl; 
        }
    }

    static void GetNodePairListFromDir(std::string aPath, NodePairList * aNodePairList) {
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir (aPath.c_str())) != NULL) {
          /* print all the files and directories within directory */
          while ((ent = readdir (dir)) != NULL) {
            if (utils::FileUtils::hasEnding(ent->d_name, "edges")){
                std::string fullPath = aPath + FILE_SEPERATOR + ent->d_name;
                std::string delimiter = " ";
                utils::FileUtils::GetNodePairListFromFile(fullPath, aNodePairList, delimiter);
            }
            if (utils::FileUtils::hasEnding(ent->d_name, "txt")){
                std::string fullPath = aPath + FILE_SEPERATOR + ent->d_name;
                std::string delimiter = "\t";
                utils::FileUtils::GetNodePairListFromFile(fullPath, aNodePairList, delimiter);
            }
            if (utils::FileUtils::hasEnding(ent->d_name, "csv")){
                std::string fullPath = aPath + FILE_SEPERATOR + ent->d_name;
                std::string delimiter = ",";
                utils::FileUtils::GetNodePairListFromFile(fullPath, aNodePairList, delimiter);
            }
            if (utils::FileUtils::hasEnding(ent->d_name, "edgelist")){
                std::string fullPath = aPath + FILE_SEPERATOR + ent->d_name;
                std::string delimiter = " ";
                utils::FileUtils::GetNodePairListFromFile(fullPath, aNodePairList, delimiter);
            }
          }
          closedir (dir);
        } else {
          /* could not open directory */
          perror ("could not open directory");
        }
        return;
    }

    static void GetCommunityFromDir(std::string aPath, Communities * aCommunities){
        DIR *dir;
        struct dirent *ent;
        std::cout << aPath << std::endl;
        if ((dir = opendir (aPath.c_str())) != NULL) {
          /* print all the files and directories within directory */
          while ((ent = readdir (dir)) != NULL) {
            if (utils::FileUtils::hasEnding(ent->d_name, "txt")){
                std::string fullPath = aPath + FILE_SEPERATOR + ent->d_name;
                std::string delimiter = "\t";
                utils::FileUtils::GetCommunityFromFile(fullPath, aCommunities, delimiter);
            }
          }
          closedir (dir);
        } else {
          /* could not open directory */
          perror ("could not open directory ");
        }
        return;
    }
};

class Resources {
public:
    static int parseLine(char* line){
        int i = strlen(line);
        while (*line < '0' || *line > '9') line++;
        line[i-3] = '\0';
        i = atoi(line);
        return i;
    }
    

    static int getVMValue(){ //Note: this value is in KB!
        FILE* file = fopen("/proc/self/status", "r");
        int result = -1;
        char line[128];
    

        while (fgets(line, 128, file) != NULL){
            if (strncmp(line, "VmSize:", 7) == 0){
                result = parseLine(line);
                break;
            }
        }
        fclose(file);
        return result;
    }

    static  int getRSSValue(){ //Note: this value is in KB!
        FILE* file = fopen("/proc/self/status", "r");
        int result = -1;
        char line[128];
    

        while (fgets(line, 128, file) != NULL){
            if (strncmp(line, "VmRSS:", 6) == 0){
                result = parseLine(line);
                break;
            }
        }
        fclose(file);
        return result;
    }
};

class Community {
public:
    void LoadCommunity(std::string aDirPath) {
        utils::FileUtils::GetCommunityFromDir(aDirPath, &myCommun);
    }
    SingleCommunity * GetCommunity(int i) {
        return &(myCommun[i]);
    }
    
    int CalcInterection(SingleCommunity * a, SingleCommunity * b) {
        int intersection = 0;
        for(unsigned int i=0; i < a->size(); i++) {
            for(unsigned int j=0; j < b->size(); j++) {
                if ((*a)[i] == (*b)[j]) {
                    intersection += 1;
                }
            }
        }
        return intersection;
    }

    void GetCommunitiesWithIntersection(long long * aOneId,
                                        long long * aTwoId,
                                        long long aThreshold ) {
        for(unsigned int i=0; i < myCommun.size(); i++) {
            for (unsigned int j=i+1; j < myCommun.size(); j++) {
                if (CalcInterection(&(myCommun[i]), &(myCommun[j]) ) > aThreshold) {
                    (*aOneId) = i;
                    (*aTwoId) = j;
                    return;
                }
            }
        }
        (*aOneId) = -1;
        (*aTwoId) = -1;
    }
    
    void GetNodesDist(std::vector<int> * dist) {
        for(unsigned int i=0; i < myCommun.size(); i++) {
            for( unsigned int j=0; j < myCommun[i].size(); j++) {
                int nId = myCommun[i][j];
                (*dist)[nId] += 1;
            }
        }
    }
    Communities myCommun;
};

} // namespace utils
} // namespace all_distance_sketch

#endif

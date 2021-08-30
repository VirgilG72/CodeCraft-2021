
#include <bits/stdc++.h>

using namespace std;

//1:commit	0:test
#if 1
#define COMMIT
#else
#define TEST
#endif
int sum = 0;

int bushu_left = 28;
int bushu_right = 25;
int qianyi_left = 30;
int qianyi_right = 10;

typedef struct svrInfo_t
{
    string svrType;
    short scores;
    short cpuCores_A;
    short cpuCores_B;
    short memorySize_A;
    short memorySize_B;
    int serverCost;
    short powerCost;

} svrInfo;

typedef struct vmInfo_t
{
    string vmType;
    short cpuCores;
    short memorySize;
    bool is2Nodes;
    int scores;
} vmInfo;

typedef enum onNode_t
{
    A,
    B,
    AB
} onNode;

typedef struct vmRunOnSvrInfo_t
{
    string vmType;
    string vmId;
    int svrId;
    short vmCores;
    short vmMemory;
    onNode on; //0:A  1:B	2:AB
} vmRunOnSvrInfo;

typedef struct notDeployVmInfo_t
{
    string vmType;
    string vmId;
    int cpuCores;
    int memorySize;
    bool is2Nodes;
    int scores;
    int pushSeq;
} notDeployVmInfo;

vector<vector<int>> outMigration;
vector<pair<int, int>> deployInfos;
vector<pair<int, int>> deployInfosForSegment;

vector<pair<int, int>> _out;

bool cmpUndeployVms(const notDeployVmInfo *a, const notDeployVmInfo *b)
{
    return a->scores > b->scores;
}
unordered_map<string, string> vmMapSvr;
// server infomation
unordered_map<string, svrInfo> svrInfos;
// server id to type
//unordered_map<int, string> svrId2Type;
// vmware infomation
unordered_map<string, vmInfo> vmInfos;
// request infomation
vector<vector<string>> reqInfos;

// Servers already purchased
int vmCounts = 0;
int svrNum = 0;
int migrationCounts = 0;
unordered_map<int, svrInfo> ownSvr;
//mappedID
unordered_map<int, int> mapSvrIDMess2Order;
unordered_map<int, int> mapSvrIDOrder2Mess;

// Number of virtual machines running on the server
//vector<int> svrRunVms;
vector<vector<string>> svrRunVms;
//string svrRunVms[10000][100]
// The virtual machine runs on the service
unordered_map<string, vmRunOnSvrInfo> vmRunOnSvr;

// Server purchased today
// unordered_map<string,int> SvrBuyToday;
vector<pair<string, int>> SvrBuyToday;

//print infomation
vector<string> out;

vector<pair<string, svrInfo> *> svrInfos_vec;

vector<pair<int, svrInfo> *> ownsvr_vec;
//vector<pair<int, int>> _out;

#ifdef TEST
//const string filePath = "C:\\Users\\Kungkaching\\Desktop\\small.txt";
const string filePath = "./training-data/training-1.txt";
int runPercent = 0;
#endif
// Cost
long long SVRCOST = 0, POWERCOST = 0, TOTALCOST = 0;

//bool cmpServerMemoryUp(const Server &server1, const Server &server2)
//{
//    return server1.memoryA>server2.memoryA;
//}
bool comp2(const pair<int, svrInfo> *a, const pair<int, svrInfo> *b)
{
    // if(a.second.cpuCores_A <= b.second.cpuCores_A&&a.second.cpuCores_B<=b.second.cpuCores_B)
    //     return true;
    // else if(a.second.cpuCores_A >= b.second.cpuCores_A&&a.second.cpuCores_B>=b.second.cpuCores_B)
    //     return false;
    // if(abs(a.second.cpuCores_A -a.second.cpuCores_B)<=abs(b.second.cpuCores_A -b.second.cpuCores_B))
    //     return true;
    // else if(abs(a.second.cpuCores_A -a.second.cpuCores_B)>=abs(b.second.cpuCores_A -b.second.cpuCores_B))
    //     return false;
    return a->second.scores < b->second.scores;
    // return (a->second.cpuCores_A + a->second.cpuCores_B) * 3 + (a->second.memorySize_A + a->second.memorySize_B) * 1 < (b->second.cpuCores_A + b->second.cpuCores_B) * 3 + (b->second.memorySize_A + b->second.memorySize_B) * 1;
    //return svrInfos[a.second.type].cpuCores_A * 0.75 + svrInfos[a.second.type].memorySize_A*0.25 < svrInfos[b.second.type].cpuCores_A * 0.75 + svrInfos[b.second.type].memorySize_A * 0.25;
    //return (1-a.second.cpuCores_A/svrInfos[a.second.type].cpuCores_A)>(1-b.second.cpuCores_A/svrInfos[b.second.type].cpuCores_A);
    //return a.second.cpuCores_A - a.second.cpuCores_B <= b.second.cpuCores_A - b.second.cpuCores_B;
}

short computeSvrScores(svrInfo &svr)
{
    return (svr.cpuCores_A + svr.cpuCores_B) * 3 + (svr.memorySize_A + svr.memorySize_B) * 1;
}

bool comp(const pair<string, svrInfo> *a, const pair<string, svrInfo> *b)
{
    return a->second.serverCost < b->second.serverCost;
}

void sortOwnSvr()
{
    // for (auto x : ownSvr) {
    // 	ownsvr_vec.push_back(x);
    // }
    sort(ownsvr_vec.begin(), ownsvr_vec.end(), comp2);
    // for (auto it = ownsvr_vec.begin(); it != ownsvr_vec.end(); ++it) {
    // 	cout << it->first << ":" << it->second.cpuCores_A << " " << it->second.cpuCores_B << endl;
    // }
    // cout << endl;
}

void sortSvr()
{
    for (auto x : svrInfos)
    {
        pair<string, svrInfo> *p1 = new pair<string, svrInfo>;
        p1->first = x.first;
        p1->second = x.second;
        //cout << x.first << endl;
        svrInfos_vec.push_back(p1);
    }
    sort(svrInfos_vec.begin(), svrInfos_vec.end(), comp);
    // for (auto it = svrInfos_vec.begin(); it != svrInfos_vec.end(); ++it) {
    // 	cout << it->first << ":" << it->second.serverCost << endl;
    // }
    // cout << svrInfos_vec[0].first << endl;
}

void genSvr(string &svrType, string &svrCpuCores, string &svrMemorySize, string &svrCost, string &svrPowerCost)
{
    string t_svrType = "";
    short t_svrCpuCores = 0, t_svrMemorySize = 0, t_svrPowerCost = 0;
    int t_svrCost = 0;
    int svrTypeSize = svrType.size() - 1;
    for (int i = 1; i < svrTypeSize; ++i)
    {
        t_svrType += svrType[i];
    }

    int svrCoreSizes = svrCpuCores.size() - 1;
    for (int i = 0; i < svrCoreSizes; ++i)
    {
        t_svrCpuCores = 10 * t_svrCpuCores + svrCpuCores[i] - '0';
    }

    int svrMemSize = svrMemorySize.size() - 1;
    for (int i = 0; i < svrMemSize; ++i)
    {
        t_svrMemorySize = 10 * t_svrMemorySize + svrMemorySize[i] - '0';
    }

    int svrCostSize = svrCost.size() - 1;
    for (int i = 0; i < svrCostSize; ++i)
    {
        t_svrCost = 10 * t_svrCost + svrCost[i] - '0';
    }

    int svrPwCostSize = svrPowerCost.size() - 1;
    for (int i = 0; i < svrPwCostSize; ++i)
    {
        t_svrPowerCost = 10 * t_svrPowerCost + svrPowerCost[i] - '0';
    }

    short t_svrCpuCores_A, t_svrCpuCores_B;
    short t_svrMemorySize_A, t_svrMemorySize_B;

    t_svrCpuCores_A = t_svrCpuCores_B = t_svrCpuCores >> 1;
    t_svrMemorySize_A = t_svrMemorySize_B = t_svrMemorySize >> 1;
    svrInfos[t_svrType] = svrInfo{t_svrType, 0, t_svrCpuCores_A, t_svrCpuCores_B,
                                  t_svrMemorySize_A, t_svrMemorySize_B,
                                  t_svrCost, t_svrPowerCost};
}

void genVm(string &vmType, string &vmCpuCores, string &vmMemorySize, string &vmIs2Nodes)
{
    string t_vmType;
    short t_vmCpuCores = 0, t_vmMemorySize = 0;
    bool t_vmIs2Nodes = 0;

    int vmTypeSize = vmType.size() - 1;
    for (int i = 1; i < vmTypeSize; ++i)
    {
        t_vmType += vmType[i];
    }

    int vmCoreSize = vmCpuCores.size() - 1;
    for (int i = 0; i < vmCoreSize; ++i)
    {
        t_vmCpuCores = t_vmCpuCores * 10 + vmCpuCores[i] - '0';
    }

    int vmMemSize = vmMemorySize.size() - 1;
    for (int i = 0; i < vmMemSize; ++i)
    {
        t_vmMemorySize = t_vmMemorySize * 10 + vmMemorySize[i] - '0';
    }

    t_vmIs2Nodes = vmIs2Nodes[0] == '1' ? 1 : 0;
    vmInfos[t_vmType] = vmInfo{t_vmType, t_vmCpuCores, t_vmMemorySize, t_vmIs2Nodes, 2*t_vmCpuCores + t_vmMemorySize};
}

// push add request
void genReq(string &repOp, string &reqVmType, string &reqId)
{
    string t_repOp, t_reqVmType, t_reqId;
    t_repOp = repOp.substr(1, repOp.size() - 1);
    t_reqVmType = reqVmType.substr(0, reqVmType.size() - 1);
    t_reqId = reqId.substr(0, reqId.size() - 1);
    reqInfos.push_back(vector<string>{t_repOp, t_reqVmType, t_reqId});
}

// push delete request
void genReq(string &repOp, string &reqId)
{
    string t_repOp, t_reqId;
    t_reqId = reqId.substr(0, reqId.size() - 1);
    t_repOp = repOp.substr(1, repOp.size() - 1);
    reqInfos.push_back(vector<string>{t_repOp, t_reqId});
}

bool selectSvr(svrInfo &svr, notDeployVmInfo *vm, int &serverId, string &vmId)
{
    short vmCores = vm->cpuCores, vmMemory = vm->memorySize, vmIs2Nodes = vm->is2Nodes;
    short &serverCoreA = svr.cpuCores_A, &serverCoreB = svr.cpuCores_B,
          &serverMemoryA = svr.memorySize_A, &serverMemoryB = svr.memorySize_B;
    if (vmIs2Nodes)
    {
        int needCores = vmCores >> 1, needMemory = vmMemory >> 1;
        if (serverCoreA >= needCores && serverMemoryA >= needMemory && serverCoreB >= needCores &&
            serverMemoryB >= needMemory)
        {
            serverCoreA -= needCores;
            serverCoreB -= needCores;
            serverMemoryA -= needMemory;
            serverMemoryB -= needMemory;

            vmRunOnSvr[vmId] = vmRunOnSvrInfo{vm->vmType, vmId, serverId, vmCores, vmMemory, AB};
            //    _out.push_back("("+to_string(mapSvrIDMess2Order[serverId])+")\n");
            //_out.push_back({serverId, 2});
            deployInfosForSegment[vm->pushSeq] = {serverId, AB};
            svr.scores = computeSvrScores(svr);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (serverCoreA >= vmCores && serverMemoryA >= vmMemory)
    {
        serverCoreA -= vmCores;
        serverMemoryA -= vmMemory;
        vmRunOnSvr[vmId] = vmRunOnSvrInfo{vm->vmType, vmId, serverId, vmCores, vmMemory, A};
        //   _out.push_back("("+to_string(mapSvrIDMess2Order[serverId])+", A)\n");
        //_out.push_back({serverId, 0});
        deployInfosForSegment[vm->pushSeq] = {serverId, A};
        svr.scores = computeSvrScores(svr);
        return true;
    }
    else if (serverCoreB >= vmCores && serverMemoryB >= vmMemory)
    {
        serverCoreB -= vmCores;
        serverMemoryB -= vmMemory;
        vmRunOnSvr[vmId] = vmRunOnSvrInfo{vm->vmType, vmId, serverId, vmCores, vmMemory, B};
        //   _out.push_back("("+to_string(mapSvrIDMess2Order[serverId])+", B)\n");
        //_out.push_back({serverId, 1});
        deployInfosForSegment[vm->pushSeq] = {serverId, B};
        svr.scores = computeSvrScores(svr);
        return true;
    }
    return false;
}

bool selectSvrSuccess(svrInfo &svr, notDeployVmInfo *vm, int &serverId, string &vmId)
{
    short vmCores = vm->cpuCores, vmMemory = vm->memorySize, vmIs2Nodes = vm->is2Nodes;
    short &serverCoreA = svr.cpuCores_A, &serverCoreB = svr.cpuCores_B,
          &serverMemoryA = svr.memorySize_A, &serverMemoryB = svr.memorySize_B;
    if (vmIs2Nodes)
    {
        int needCores = vmCores / 2, needMemory = vmMemory / 2;
        if (serverCoreA >= needCores && serverMemoryA >= needMemory && serverCoreB >= needCores &&
            serverMemoryB >= needMemory)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (serverCoreA >= vmCores && serverMemoryA >= vmMemory)
    {
        return true;
    }
    else if (serverCoreB >= vmCores && serverMemoryB >= vmMemory)
    {
        return true;
    }
    return false;
}

int createVM(notDeployVmInfo *createVmInfo)
{
    string _reqId = createVmInfo->vmId;
    //vmInfo vm = vmInfos[_reqVmType];
    int success = -1;

    int low = 0, high = svrNum;
    int mid = 0;
    int lastSuccess = -1;
    int curSuccess = -1;
    while (low < high)
    {
        mid = (low + high) >> 1;
        int tmp = ownsvr_vec[mid]->first;
        svrInfo &svr = ownSvr[tmp];
        curSuccess = selectSvrSuccess(svr, createVmInfo, tmp, _reqId);

        if (curSuccess == 0)
        {
            int left = max(mid - bushu_left, low);
            int right = min(mid + bushu_right, high);
            for (int m = left; m < right; m++)
            {
                int tmp = ownsvr_vec[m]->first;
                svrInfo &svr = ownSvr[tmp];
                curSuccess = selectSvrSuccess(svr, createVmInfo, tmp, _reqId);
                if (curSuccess)
                {
                    mid = m;
                    break;
                }
            }
        }

        if (lastSuccess == 1 && curSuccess == 0)
        {
            for (int i = mid + 1; i <= high; i++)
            {
                tmp = ownsvr_vec[i]->first;
                svrInfo &svr = ownSvr[tmp];
                if (selectSvr(svr, createVmInfo, tmp, _reqId))
                {
                    vmCounts++;
                    svrRunVms[tmp].push_back(_reqId);
                    success = 1;
                    ownsvr_vec[i]->second = svr;
                    return success;
                }
            }
        }

        if (curSuccess == 1)
        { //success
            high = mid;
            lastSuccess = curSuccess;
        }
        else
        { //failed
            low = mid + 1;
            lastSuccess = curSuccess;
        }
    }

    for (int i = 0; i < svrNum; i++)
    {
        //svrInfo &svr = ownSvr[i];
        int tmp = ownsvr_vec[i]->first;
        svrInfo &svr = ownSvr[tmp];
        if (selectSvr(svr, createVmInfo, tmp, _reqId))
        {
            vmCounts++;
            svrRunVms[tmp].push_back(_reqId);
            success = 1;
            ownsvr_vec[i]->second = svr;
            return success;
        }
    }

    return success;
}
void delVM(vector<string> &delVmInfo)
{
    string t_vmId = delVmInfo[1];
    // assert(vmRunOnSvr.find(t_vmId) != vmRunOnSvr.end()); //等于end时代表找不到

    vmRunOnSvrInfo t_vmInfo = vmRunOnSvr[t_vmId];

    int t_serverId = t_vmInfo.svrId;
    //svrInfo *server;
    vmCounts--;
    //    svrRunVms[t_serverId]--;
    svrRunVms[t_serverId].erase(remove(svrRunVms[t_serverId].begin(), svrRunVms[t_serverId].end(), t_vmId),
                                svrRunVms[t_serverId].end());

    svrInfo &server = ownSvr[t_serverId];
    if (t_vmInfo.on == AB)
    {
        short cores = t_vmInfo.vmCores >> 1, memory = t_vmInfo.vmMemory >> 1;
        server.cpuCores_A += cores;
        //(*server).cpuCores_A += cores;
        server.cpuCores_B += cores;
        server.memorySize_A += memory;
        server.memorySize_B += memory;
        server.scores = computeSvrScores(server);
    }
    else
    {
        short cores = t_vmInfo.vmCores, memory = t_vmInfo.vmMemory;
        if (t_vmInfo.on == A)
        {
            server.cpuCores_A += cores;
            server.memorySize_A += memory;
            server.scores = computeSvrScores(server);
        }
        else
        {
            server.cpuCores_B += cores;
            server.memorySize_B += memory;
            server.scores = computeSvrScores(server);
        }
    }

    int size = ownsvr_vec.size();
    for (int i = 0; i < size; i++)
    {
        if (ownsvr_vec[i]->first == t_serverId)
        {
            ownsvr_vec[i]->second = server;
            break;
        }
    }
}

//TODO
void purchase()
{
    //	int purchaseNum = 0;
    //	for(auto it=SvrBuyToday.begin();it!=SvrBuyToday.end();it++){
    //    	purchaseNum +=it->second;
    //    }
    string s = "(purchase, " + to_string(SvrBuyToday.size()) + ")\n";
    out.push_back(s);

    // for(auto it=SvrBuyToday.begin();it!=SvrBuyToday.end();it++){
    // 	string s = "("+it->first+", "+ to_string(it->second) +")\n";
    // 	out.push_back(s);
    // }
    for (auto iter = SvrBuyToday.begin(); iter != SvrBuyToday.end(); ++iter)
    {
        string s = "(" + iter->first + ", " + to_string(iter->second) + ")\n";
        out.push_back(s);
    }
}

//TODO
void migration()
{
    int migrationTimes = outMigration.size();

    string s = "(migration, " + to_string(migrationTimes) + ")\n";
    out.push_back(s);

    for (int i = 0; i < migrationTimes; ++i)
    {
        int vmId = outMigration[i][0];
        int svrId = mapSvrIDMess2Order[outMigration[i][1]];
        int onNode = outMigration[i][2];
        if (onNode == 2)
        { //AB
            s = "(" + to_string(vmId) + ", " + to_string(svrId) + ")\n";
        }
        else if (onNode == 0)
        { //A
            s = "(" + to_string(vmId) + ", " + to_string(svrId) + ", A)\n";
        }
        else
        { //B
            s = "(" + to_string(vmId) + ", " + to_string(svrId) + ", B)\n";
        }
        out.push_back(s);
    }
}

void addSvr(notDeployVmInfo *createVmInfo)
{
    string _reqVmType = createVmInfo->vmType;
    string svrType;

    for (auto it = svrInfos_vec.begin(); it != svrInfos_vec.end(); ++it)
    {
        if (vmMapSvr[_reqVmType] != "")
        {
            svrType = vmMapSvr[_reqVmType];
            break;
        }

        vmInfo vm = vmInfos[_reqVmType];
        svrInfo svr = svrInfos[(*it)->first];
        short vmCores = vm.cpuCores, vmMemory = vm.memorySize, vmIs2Nodes = vm.is2Nodes;
        short serverCoreA = svr.cpuCores_A, serverCoreB = svr.cpuCores_B,
              serverMemoryA = svr.memorySize_A, serverMemoryB = svr.memorySize_B;
        if (vmIs2Nodes)
        {
            short needCores = vmCores >> 1, needMemory = vmMemory >> 1;
            if (serverCoreA >= needCores && serverCoreB >= needCores && serverMemoryAR >= needMemory &&
                serverMemoryB >= needMemory)
            {
                svrType = (*it)->first;
                vmMapSvr[_reqVmType] = svrType;
                break;
            }
        }
        else if (serverCoreA >= vmCores && serverMemoryA >= vmMemory)
        {
            svrType = (*it)->first;
            vmMapSvr[_reqVmType] = svrType;
            break;
        }
        else if (serverCoreB >= vmCores && serverMemoryB >= vmMemory)
        {
            svrType = (*it)->first;
            vmMapSvr[_reqVmType] = svrType;
            break;
        }
        continue;
    }
    //string svrType = svrInfos_vec[0].first;
    //cout << svrType << endl;
    ownSvr[svrNum++] = svrInfos[svrType];
    int t_svrId = svrNum - 1;
    string t_vmId = createVmInfo->vmId;
    svrInfo &svr = ownSvr[t_svrId];

    //ownsvr_vec.emplace_back((svrNum - 1), svr);
    // int n = rand() % svrInfos.size();
    // string svrType = svrId2Type[n];
    // ownSvr[svrNum++] = svrInfos[svrType];

    selectSvr(svr, createVmInfo, t_svrId, t_vmId);
    vmCounts++;
    svrRunVms[t_svrId].push_back(t_vmId);
    //ownsvr_vec[svrNum - 1]->second = svr;
    pair<int, svrInfo> *p = new pair<int, svrInfo>;
    p->first = t_svrId;
    p->second = svr;
    ownsvr_vec.emplace_back(p);
    //do not need to modify below
    bool isBuyType = 0;

    for (auto iter = SvrBuyToday.begin(); iter != SvrBuyToday.end(); ++iter)
    {
        if (iter->first == svrType)
        { //先去判断是否是已经购买的类型，若是，则个数+1
            ++iter->second;
            isBuyType = 1;
            break;
        }
    }
    if (isBuyType == 1)
    {
        int i = 1;
        while (i)
        {
            int Order2Mess = mapSvrIDOrder2Mess[svrNum - 1 - i]; //先从order那列从下到上遍历，找到对应左边的那列mess
            if (ownSvr[Order2Mess].svrType !=
                svrType)
            {                                                //如果新添加的类型与遍历选中的类型不同
                ++mapSvrIDMess2Order[Order2Mess];            //遍历选中的mess对应的order序号+1
                mapSvrIDOrder2Mess[svrNum - i] = Order2Mess; ///（映射关系）遍历选中的mess对应的order设为遍历选中的mess
            }
            else
            {                                                        //如果新添加的类型与遍历选中的类型相同
                mapSvrIDMess2Order[svrNum - 1] = svrNum - 1 - i + 1; //则直接在order那列的相同类型最后一个位置+1即为对应的Mess2Order
                mapSvrIDOrder2Mess[svrNum - 1 - i + 1] = svrNum - 1; //（映射关系）
                break;
            }
            ++i;
        }
    }
    else
    {                                                //若不是已经购买的类型，注意下标从零开始
        mapSvrIDMess2Order[svrNum - 1] = svrNum - 1; //则直接在mess列最后添加
        mapSvrIDOrder2Mess[svrNum - 1] = svrNum - 1; //（映射关系）
        SvrBuyToday.push_back({svrType, 1});
    }

    SVRCOST += svrInfos[svrType].serverCost;
}

void pushdeployInfos()
{
    for (auto iter = deployInfos.begin(); iter != deployInfos.end(); ++iter)
    {
        int serverId = iter->first;
        int onNode = iter->second;
        string s;
        if (onNode == 2)
        { //AB
            s = "(" + to_string(mapSvrIDMess2Order[serverId]) + ")\n";
        }
        else if (onNode == 0)
        { //A
            s = "(" + to_string(mapSvrIDMess2Order[serverId]) + ", A)\n";
        }
        else
        { //B
            s = "(" + to_string(mapSvrIDMess2Order[serverId]) + ", B)\n";
        }
        out.push_back(s);
    }
}

void dealReqSegments(vector<vector<string>> &reqSegments)
{
    if (reqSegments.size() == 0)
        return;
    vector<notDeployVmInfo *> reqVmInfos;
    int size = 0;
    deployInfosForSegment.clear();
    for (auto reqSegment : reqSegments)
    {
        string reqVmType = reqSegment[1], reqVmId = reqSegment[2];
        int cpuCores = vmInfos[reqVmType].cpuCores;
        int memorySize = vmInfos[reqVmType].memorySize;
        int scores = vmInfos[reqVmType].scores;
        notDeployVmInfo *t_notDeployVmInfo = new notDeployVmInfo{reqVmType, reqVmId, cpuCores, memorySize, vmInfos[reqVmType].is2Nodes, scores, size++};

        reqVmInfos.push_back(t_notDeployVmInfo);
    }
    reqSegments.clear();
    deployInfosForSegment.resize(size);
    sort(reqVmInfos.begin(), reqVmInfos.end(), cmpUndeployVms);
    sortOwnSvr();
    for (auto reqVmInfo : reqVmInfos)
    {
        if (createVM(reqVmInfo) == -1)
        {
            addSvr(reqVmInfo);
        }
    }
    //cout << "size: " <<size<<endl;
    for (int i = 0; i < size; ++i)
    {
        deployInfos.push_back(deployInfosForSegment[i]);
    }
}

int tryAddLoad(svrInfo &svr, vmRunOnSvrInfo &vm, int &serverId)
{
    short vmCores = vm.vmCores, vmMemory = vm.vmMemory, vmIs2Nodes = vm.on;
    string vmId = vm.vmId;
    short &serverCoreA = svr.cpuCores_A, &serverCoreB = svr.cpuCores_B,
          &serverMemoryA = svr.memorySize_A, &serverMemoryB = svr.memorySize_B;
    if (vmIs2Nodes == AB)
    {
        short needCores = vmCores >> 1, needMemory = vmMemory >> 1;
        if (serverCoreA >= needCores && serverMemoryA >= needMemory && serverCoreB >= needCores &&
            serverMemoryB >= needMemory)
        {
            serverCoreA -= needCores;
            serverCoreB -= needCores;
            serverMemoryA -= needMemory;
            serverMemoryB -= needMemory;
            svr.scores = computeSvrScores(svr);
            vmRunOnSvr[vmId] = vmRunOnSvrInfo{vm.vmType, vmId, serverId, vmCores, vmMemory, AB};
            //    _out.push_back("("+to_string(mapSvrIDMess2Order[serverId])+")\n");
            //            _out.push_back({serverId,2});
            return 2; //AB
        }
        else
        {
            return -1;
        }
    }
    else if (serverCoreA >= vmCores && serverMemoryA >= vmMemory)
    {
        serverCoreA -= vmCores;
        serverMemoryA -= vmMemory;
        vmRunOnSvr[vmId] = vmRunOnSvrInfo{vm.vmType, vmId, serverId, vmCores, vmMemory, A};
        svr.scores = computeSvrScores(svr);
        //   _out.push_back("("+to_string(mapSvrIDMess2Order[serverId])+", A)\n");
        //        _out.push_back({serverId,0});
        return 0;
    }
    else if (serverCoreB >= vmCores && serverMemoryB >= vmMemory)
    {
        serverCoreB -= vmCores;
        serverMemoryB -= vmMemory;
        vmRunOnSvr[vmId] = vmRunOnSvrInfo{vm.vmType, vmId, serverId, vmCores, vmMemory, B};
        svr.scores = computeSvrScores(svr);
        //   _out.push_back("("+to_string(mapSvrIDMess2Order[serverId])+", B)\n");
        //        _out.push_back({serverId,1});
        return 1;
    }
    return -1;
}

void delLoad(svrInfo &ownSvr, vmRunOnSvrInfo &vm)
{
    svrInfo &server = ownSvr;
    short cpu = vm.vmCores, memory = vm.vmMemory, onNode = vm.on;
    if (onNode == AB)
    {
        cpu = cpu >> 1, memory = memory >> 1;
        server.cpuCores_A += cpu;
        server.cpuCores_B += cpu;
        server.memorySize_A += memory;
        server.memorySize_B += memory;
        server.scores = computeSvrScores(server);
    }
    else
    {
        if (onNode == A)
        {
            server.cpuCores_A += cpu;
            server.memorySize_A += memory;
            server.scores = computeSvrScores(server);
        }
        else
        {
            server.cpuCores_B += cpu;
            server.memorySize_B += memory;
            server.scores = computeSvrScores(server);
        }
    }
    return;
}

int migrationSuccess(svrInfo &svr, vmRunOnSvrInfo &vm)
{
    short vmCores = vm.vmCores, vmMemory = vm.vmMemory, vmIs2Nodes = vm.on;
    string vmId = vm.vmId;
    short serverCoreA = svr.cpuCores_A, serverCoreB = svr.cpuCores_B,
          serverMemoryA = svr.memorySize_A, serverMemoryB = svr.memorySize_B;
    if (vmIs2Nodes == AB)
    {
        short needCores = vmCores >> 1, needMemory = vmMemory >> 1;
        if (serverCoreA >= needCores && serverMemoryA >= needMemory && serverCoreB >= needCores &&
            serverMemoryB >= needMemory)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if (serverCoreA >= vmCores && serverMemoryA >= vmMemory)
    {
        return 1;
    }
    else if (serverCoreB >= vmCores && serverMemoryB >= vmMemory)
    {
        return 1;
    }
    return 0;
}

void tryMigration()
{
    int migrationNum = vmCounts * 5 / 1000;
    sum = sum + migrationNum;
    if (migrationNum <= 0)
        return;

    int svrMaxSize = ownsvr_vec.size();
    for (int j = (svrMaxSize - 1); j >= 0; j--)
    {
        /* code */
        int svrIDForFewVm = ownsvr_vec[j]->first; //负载最少的服务器ID
        int vmMaxSize = svrRunVms[svrIDForFewVm].size();
        if (vmMaxSize == 0)
            continue;
        for (int k = 0; k < vmMaxSize; ++k)
        { /* code */
            string vmId = svrRunVms[svrIDForFewVm][k];
            vmRunOnSvrInfo vm = vmRunOnSvr[vmId];
            int low = 0, high = j;
            int mid = 0;
            int lastSuccess = -1;
            int curSuccess = -1;
            // int lastMid = 0;
            int flag = 0;
            while (low < high)
            {
                mid = (low + high) >> 1;
                int svrIdForManyVm = ownsvr_vec[mid]->first;
                curSuccess = migrationSuccess(ownSvr[svrIdForManyVm], vm);

                if (curSuccess == 0)
                {
                    int left = max(mid - qianyi_left, low);
                    int right = min(mid + qianyi_right, high);
                    for (int m = left; m < right; m++)
                    {
                        svrIdForManyVm = ownsvr_vec[m]->first;
                        curSuccess = migrationSuccess(ownSvr[svrIdForManyVm], vm);
                        if (curSuccess)
                        {
                            mid = m;
                            break;
                        }
                    }
                }

                if (lastSuccess == 1 && curSuccess == 0)
                {
                    for (int i = mid + 1; i <= high; ++i)
                    {
                        int svrIdForManyVm = ownsvr_vec[i]->first;
                        int onNode = tryAddLoad(ownSvr[svrIdForManyVm], vm, svrIdForManyVm);
                        if (onNode != -1)
                        {
                            vector<int> migrationInfo;
                            migrationInfo.push_back(atoi(vmId.c_str()));
                            migrationInfo.push_back(svrIdForManyVm);
                            migrationInfo.push_back(onNode);
                            outMigration.push_back(migrationInfo);
                            svrRunVms[svrIdForManyVm].push_back(vmId);
                            delLoad(ownSvr[svrIDForFewVm], vm);
                            svrRunVms[svrIDForFewVm].erase(
                                remove(svrRunVms[svrIDForFewVm].begin(), svrRunVms[svrIDForFewVm].end(), vmId),
                                svrRunVms[svrIDForFewVm].end());

                            vmMaxSize--;
                            //iii--;
                            k--;
                            migrationNum--;
                            migrationCounts++;
                            if (migrationNum <= 0)
                                return;
                            flag = 1;
                            break;
                        }
                    }
                    if (flag == 1)
                        break;
                }

                if (curSuccess == 1)
                { //success
                    high = mid;
                    // lastMid = mid;
                    lastSuccess = curSuccess;
                }
                else
                { //failed
                    low = mid + 1;
                    // lastMid = mid;
                    lastSuccess = curSuccess;
                }
            }
        }
    }
}
//int i_best = 0;
//TODO
void decision(int &day)
{
    SvrBuyToday.clear();
    deployInfos.clear();
    _out.clear();
    outMigration.clear();
    sortOwnSvr();
    //tryMigration();

    //sortOwnSvr();
    vector<vector<string>> reqSegments;
    for (auto req : reqInfos)
    {
        //i = 0;
        int opType = req.size() == 3 ? 1 : 0;
        if (opType)
        {
            // while (createVM(req) == -1)
            // {
            //     addSvr(req);
            // }
            reqSegments.push_back(req);
        }
        else
        {
            dealReqSegments(reqSegments);
            delVM(req);
        }
    }
    dealReqSegments(reqSegments);

    purchase();
    migration();
    pushdeployInfos();
}

void todaySvrPowerCost()
{
    for (int i = 0; i < svrNum; ++i)
    {
        if (svrRunVms[i].size() != 0)
        {
            POWERCOST += ownSvr[i].powerCost;
        }
    }
}

int main()
{
#ifdef TEST
    clock_t start, end;
    start = clock();
    int seed = 20;
    srand(seed);
    freopen(filePath.c_str(), "rb", stdin);
#endif
    int svrCount;
    string svrType, svrCpuCores, svrMemorySize, svrCost, svrPowerCost;
    scanf("%d", &svrCount);
    for (int i = 0; i < svrCount; ++i)
    {
        cin >> svrType >> svrCpuCores >> svrMemorySize >> svrCost >> svrPowerCost;
        genSvr(svrType, svrCpuCores, svrMemorySize, svrCost, svrPowerCost);
    }

    int vmNum = 0;
    cin >> vmNum;
    string vmType, vmCpuCores, vmMemorySize, vmIs2Nodes;
    for (int i = 0; i < vmNum; ++i)
    {
        cin >> vmType >> vmCpuCores >> vmMemorySize >> vmIs2Nodes;
        genVm(vmType, vmCpuCores, vmMemorySize, vmIs2Nodes);
    }

    int reqDays = 0, dayReqNum = 0;
    scanf("%d", &reqDays);
    int k = 0;//初始day
    scanf("%d", &k);
    string repOp, reqVmType, reqId;

    //  buySvr();  //TODO
    svrRunVms.resize(10000);

    sortSvr();

    for (int d = 0; d < reqDays; ++d)
    {
        scanf("%d", &dayReqNum);
        reqInfos.clear();
        for (int i = 0; i < dayReqNum; ++i)
        {
            cin >> repOp;
            if (repOp[1] == 'a')
            {
                cin >> reqVmType >> reqId;
                genReq(repOp, reqVmType, reqId);
            }
            else
            {
                cin >> reqId;
                genReq(repOp, reqId);
            }
        }
        decision(d); //TODO
#ifdef TEST
        todaySvrPowerCost();
        int curPercent = (d + 1) * 100 / reqDays;
        if (curPercent > runPercent)
        {
            runPercent = curPercent;
            printf("\rcomplete %3d%%", runPercent);
        }
#endif
    }

#ifdef COMMIT
    for (auto &o : out)
        cout << o;
#endif

#ifdef TEST
    fclose(stdin);
    end = clock();
    TOTALCOST = SVRCOST + POWERCOST;
    printf("\n\n----------------------------\n");
    printf("Run time: %f s \n", double(end - start) / CLOCKS_PER_SEC);
    printf("Server cost: %lld \n", SVRCOST);
    printf("Power cost: %lld \n", POWERCOST);
    printf("Total cost: %lld \n", TOTALCOST);
    cout << "migrationCounts: " << migrationCounts << endl;
    cout << "sum: " <<sum<< endl;
#endif
    return 0;
}

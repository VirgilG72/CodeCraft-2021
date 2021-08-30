#include <bits/stdc++.h>

using namespace std;
//1:commit	0:test
#if 0
#define COMMIT
#else
#define TEST
#endif
#ifdef TEST
const string filePath = "./training-data/training-1.txt";
#endif
const double INF = 1e9;
double svrRate_vmRate_coef = 3;
double windowRate_svrRate_coef = 2.55; //2.55
double cpuMemoryBalance_coef = 1;
double svrCost_coef = 10;     //10
double svrPowerCost_coef = 6; //6 4 3

double balanceNodeA_coef = 5; //3:1;5:1;
double balanceNodeB_coef = 1;
// double balanceNodeAB_coef = 1;
double remainCpuAndMemory_coef = 0.4;
double svrRunVmNums_coef = 3.1;
double svrRemainRate_coef = 2; //1

double totalRes_coef = 0.005; //0.005

double lowCpu_coef = 2;
double lowMemory_coef = 40;
double lowCpuFor2Nodes_coef = 2;
double lowMemoryFor2Nodes_coef = 40;
long long SVRCOST = 0, POWERCOST = 0, TOTALCOST = 0;

//----------------------------迁移使用的变量-------------------------------
int migrationNum = 0;

//-----------------------------可供购买的服务器信息-----------------------------
double AvgPowerCost = 0;
double AvgSvrCost = 0;
double TotalDay = 0;
typedef struct svrInfo_t
{
    int cpuCores_A;
    int cpuCores_B;
    int memorySize_A;
    int memorySize_B;
    int totalRes;
    int serverCost;
    int powerCost;

} svrInfo;
unordered_map<string, svrInfo> svrInfos;
vector<string> seq2SvrType; //test
int max_seq = 0;            //test
void genSvr(string &svrType, string &svrCpuCores, string &svrMemorySize, string &svrCost, string &svrPowerCost, int seq)
{
    string t_svrType = "";
    int t_svrCpuCores = 0, t_svrMemorySize = 0, t_svrPowerCost = 0;
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
    int t_svrCpuCores_A, t_svrCpuCores_B;
    int t_svrMemorySize_A, t_svrMemorySize_B;
    t_svrCpuCores_A = t_svrCpuCores_B = t_svrCpuCores >> 1;
    t_svrMemorySize_A = t_svrMemorySize_B = t_svrMemorySize >> 1;
    seq2SvrType[seq] = t_svrType;
    AvgSvrCost += t_svrCost;
    AvgPowerCost += t_svrPowerCost;
    svrInfos[t_svrType] = svrInfo{t_svrCpuCores_A, t_svrCpuCores_B,
                                  t_svrMemorySize_A, t_svrMemorySize_B,
                                  t_svrCpuCores_A + t_svrCpuCores_B + t_svrMemorySize_A + t_svrMemorySize_B,
                                  t_svrCost, t_svrPowerCost};
}

//----------------------------可供部署的虚拟机信息--------------------------------
typedef struct vmInfo_t
{
    int cpuCores;
    int memorySize;
    bool is2Nodes;
} vmInfo;
unordered_map<string, vmInfo> vmInfos;
void genVm(string &vmType, string &vmCpuCores, string &vmMemorySize, string &vmIs2Nodes)
{
    string t_vmType;
    int t_vmCpuCores = 0, t_vmMemorySize = 0;
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
    vmInfos[t_vmType] = vmInfo{t_vmCpuCores, t_vmMemorySize, t_vmIs2Nodes};
}

//-------------------------------生成所有天的请求------------------------------------
vector<vector<vector<string>>> reqInfos;
int K; //初始你只知晓前K天的数据。
void genReq(string &repOp, string &reqVmType, string &reqId, int day)
{
    string t_repOp, t_reqVmType, t_reqId;
    t_repOp = repOp.substr(1, repOp.size() - 2);
    t_reqVmType = reqVmType.substr(0, reqVmType.size() - 1);
    t_reqId = reqId.substr(0, reqId.size() - 1);
    reqInfos[day].push_back(vector<string>{t_repOp, t_reqVmType, t_reqId});
}
void genReq(string &repOp, string &reqId, int day)
{
    string t_repOp, t_reqId;
    t_reqId = reqId.substr(0, reqId.size() - 1);
    t_repOp = repOp.substr(1, repOp.size() - 2);
    reqInfos[day].push_back(vector<string>{t_repOp, t_reqId});
}

//------------------------------未部署的虚拟机的信息------------------------------------
typedef struct notDeployVmInfo_t
{
    string vmId;
    int cpuCores;
    int memorySize;
    int scores;
    bool is2Nodes;
    int pushSeq;
} notDeployVmInfo;
vector<pair<int, int>> deployInfosForSegment;
unordered_map<int, int> mapSvrIDMess2Order;
unordered_map<int, int> mapSvrIDOrder2Mess;

//-------------------------------跑起来的虚拟机的信息------------------------------------
typedef enum onNode_t
{
    A,
    B,
    AB
} onNode;
typedef struct vmRunOnSvrInfo_t
{
    string vmId;
    int svrId;
    int vmCores;
    int vmMemory;
    onNode on; //0:A  1:B	2:AB
} vmRunOnSvrInfo;
unordered_map<string, vmRunOnSvrInfo> vmRunOnSvr;
int vmNum = 0;

//-------------------------已经购买的服务器的信息-------------------------------------
typedef struct ownSvrInfo_t
{
    int svrId;
    string svrType;
    int remainCpuCores_A;
    int remainCpuCores_B;
    int totalCpuA;
    int totalCpuB;
    int totalCpu;
    int remainMemorySize_A;
    int remainMemorySize_B;
    int totalMemoryA;
    int totalMemoryB;
    int totalMemory;
    int totalRes;
    int powerCost;
    vector<string> svrRunVms; //服务器运行的虚拟机的ID
} ownSvrInfo;
int svrNum = 0;
vector<ownSvrInfo> ownSvr;

//------------------做决策(购买，部署，迁移，预测等)-----------------------------------
vector<vector<string>> out; //输出信息
vector<pair<int, int>> deployInfos;
vector<pair<string, int>> SvrBuyToday;
typedef struct bestSvrForVm_t
{
    double loss;
    int svrId;
    onNode on;
} bestSvrForVm;

typedef struct bestSvrForVmToMigration_t
{
    double loss;
    int svrId;
    onNode on;
} bestSvrForVmToMigration;
vector<vector<int>> migrationInfos;

typedef struct SlideWindowInfo_t
{
    int totalVmsNum;
    int twoNodesNum;
    int needCpu;
    int needMemory;
} SlideWindowInfo;

SlideWindowInfo slideWindowInfo;
unordered_map<string, string> vmId2Type;

//TODO
void pushPurchaseInfos(int &day)
{
    string s = "(purchase, " + to_string(SvrBuyToday.size()) + ")\n";
    out[day].push_back(s);
    for (auto iter = SvrBuyToday.begin(); iter != SvrBuyToday.end(); ++iter)
    {
        string s = "(" + iter->first + ", " + to_string(iter->second) + ")\n";
        out[day].push_back(s);
    }
}

void pushMigrationInfos(int &day)
{
    int migrationTimes = migrationInfos.size();
    string s = "(migration, " + to_string(migrationTimes) + ")\n";
    out[day].push_back(s);
    for (int i = 0; i < migrationTimes; ++i)
    {
        int vmId = migrationInfos[i][0];
        int svrId = mapSvrIDMess2Order[migrationInfos[i][1]];
        int onNode = migrationInfos[i][2];
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
        out[day].push_back(s);
    }
}

void pushDeployInfos(int &day)
{
    for (auto iter = deployInfos.begin(); iter != deployInfos.end(); ++iter)
    {
        int serverId = iter->first;
        int onNode = iter->second;
        string s;
        if (onNode == AB)
        { //AB
            s = "(" + to_string(mapSvrIDMess2Order[serverId]) + ")\n";
        }
        else if (onNode == A)
        { //A
            s = "(" + to_string(mapSvrIDMess2Order[serverId]) + ", A)\n";
        }
        else
        { //B
            s = "(" + to_string(mapSvrIDMess2Order[serverId]) + ", B)\n";
        }
        out[day].push_back(s);
    }
}
void Order_Mess_MAP(string &svrType)
{
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
            if (ownSvr[Order2Mess].svrType != svrType)
            {                                                //如果新添加的类型与遍历选中的类型不同
                mapSvrIDMess2Order[Order2Mess]++;            //遍历选中的mess对应的order序号+1
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
}

bool overLoad(notDeployVmInfo &vm, svrInfo &svr)
{
    if (vm.is2Nodes)
    {
        if (svr.cpuCores_A >= vm.cpuCores / 2 && svr.cpuCores_B >= vm.cpuCores / 2 &&
            svr.memorySize_A >= vm.memorySize / 2 && svr.memorySize_B >= vm.memorySize / 2)
            return 0;
        else
            return 1;
    }
    else if (svr.cpuCores_A >= vm.cpuCores && svr.memorySize_A >= vm.memorySize)
        return 0;
    else
        return 1;
}

string getBestSvrToBuy(notDeployVmInfo &createVmInfo, int &start, int &end)
{
    // string svrType = seq2SvrType[rand() % max_seq];
    string svrType;
    int needCpu = createVmInfo.cpuCores;
    int needMemory = createVmInfo.memorySize;
    double vmRate = needCpu * 1.0 / needMemory; //若大于1，则代表虚拟机的CPU>Memory；若小于1，则代表虚拟机的CPU<Memory
    double minLoss = INF;
    double loss;
    for (auto it = svrInfos.begin(); it != svrInfos.end(); it++)
    {
        if (overLoad(createVmInfo, it->second) == 1) //如果超出负载，则跳过
            continue;
        double totalCpu = it->second.cpuCores_A * 2;                                          //服务器的CPU核数
        double totalMemory = it->second.memorySize_A * 2;                                     //服务器的内存数
        double svrRate = totalCpu / totalMemory;                                              //若大于1，则代表服务器的CPU>Memory；若小于1，则代表服务器的CPU<Memory
        double cpuMemoryBalance = abs(totalCpu - totalMemory) / (totalCpu + totalMemory) / 2; //尽可能使服务器的CPU与Memory相近
        double svrCostRate = it->second.serverCost / AvgSvrCost;                              //使svrCost尽可能接近1
        double svrPowerCostRate = it->second.powerCost / AvgPowerCost;                        //使powerCost尽可能接近1

        if (slideWindowInfo.needCpu > 0 && slideWindowInfo.needMemory > 0 && slideWindowInfo.totalVmsNum > 0) //当天del数小于add数的情况
        {
            double windowRate = slideWindowInfo.needCpu * 1.0 / slideWindowInfo.needMemory;    //若大于1，则代表滑动窗口的CPU>Memory；若小于1，则代表滑动窗口的CPU<Memory
            double nodeRate = slideWindowInfo.twoNodesNum * 1.0 / slideWindowInfo.totalVmsNum; //代表双结点数
            loss = abs(svrRate - vmRate) * svrRate_vmRate_coef +                               //尽可能使虚拟机的Rate与服务器的Rate相近
                   abs(windowRate - svrRate) * windowRate_svrRate_coef +                       //尽可能使当天的Rate与服务器的Rate相近
                   cpuMemoryBalance * nodeRate * cpuMemoryBalance_coef +                       //若双结点比较多，则cpuMemoryBalance的值要求小；若单结点比较多，则cpuMemoryBalance的值可大可小
                   svrCostRate * start / TotalDay * svrCost_coef +                             //前期选择服务器硬件成本大的，后期选择服务器硬件成本小的
                   svrPowerCostRate * (TotalDay - start) / TotalDay * svrPowerCost_coef;       //前期选择服务器能源成本小的，后期选择服务器能源成本大的
        }
        else //当天del数大于add数的情况
        {
            loss = abs(svrRate - vmRate) * svrRate_vmRate_coef +                         //尽可能使虚拟机的Rate与服务器的Rate相近
                   svrCostRate * start / TotalDay * svrCost_coef +                       //前期选择服务器硬件成本大的，后期选择服务器硬件成本小的
                   svrPowerCostRate * (TotalDay - start) / TotalDay * svrPowerCost_coef; //前期选择服务器能源成本小的，后期选择服务器能源成本大的
        }
        if (loss < minLoss)
        {
            minLoss = loss;
            svrType = it->first;
        }
    }
    return svrType;
}

void addSvr(notDeployVmInfo &createVmInfo, int &start, int &end)
{
    string svrType = getBestSvrToBuy(createVmInfo, start, end);
    svrInfo svr = svrInfos[svrType];
    ownSvr.push_back(ownSvrInfo{svrNum, svrType,
                                svr.cpuCores_A, svr.cpuCores_B,
                                svr.cpuCores_A, svr.cpuCores_B,
                                svr.cpuCores_A * 2,
                                svr.memorySize_A, svr.memorySize_B,
                                svr.memorySize_A, svr.memorySize_B,
                                svr.memorySize_A * 2,
                                svr.cpuCores_A * 2 + svr.memorySize_A * 2,
                                svr.powerCost});
    svrNum++;
    Order_Mess_MAP(svrType); //部署映射实现
    SVRCOST += svrInfos[svrType].serverCost;
}

void selectSvr(ownSvrInfo &svr, notDeployVmInfo &vm, onNode on)
{
    if (on == AB)
    {
        int needCores = vm.cpuCores / 2;
        int needMemory = vm.memorySize / 2;
        svr.remainCpuCores_A -= needCores;
        svr.remainCpuCores_B -= needCores;
        svr.remainMemorySize_A -= needMemory;
        svr.remainMemorySize_B -= needMemory;
        vmRunOnSvr[vm.vmId] = vmRunOnSvrInfo{vm.vmId, svr.svrId, vm.cpuCores, vm.memorySize, AB};
        deployInfosForSegment[vm.pushSeq] = {svr.svrId, AB};
        return;
    }
    if (on == A)
    {
        svr.remainCpuCores_A -= vm.cpuCores;
        svr.remainMemorySize_A -= vm.memorySize;
        vmRunOnSvr[vm.vmId] = vmRunOnSvrInfo{vm.vmId, svr.svrId, vm.cpuCores, vm.memorySize, A};
        deployInfosForSegment[vm.pushSeq] = {svr.svrId, A};
        return;
    }
    if (on == B)
    {
        svr.remainCpuCores_B -= vm.cpuCores;
        svr.remainMemorySize_B -= vm.memorySize;
        vmRunOnSvr[vm.vmId] = vmRunOnSvrInfo{vm.vmId, svr.svrId, vm.cpuCores, vm.memorySize, B};
        deployInfosForSegment[vm.pushSeq] = {svr.svrId, B};
        return;
    }
}

double computeLossForBestSvrForVm(double remainCpuCores_A, double remainCpuCores_B,
                                  double remainMemorySize_A, double remainMemorySize_B,
                                  double svrRunVmNums, int &VMneedCpu, int &VMneedMemory, ownSvrInfo &svr, onNode onNode, int &start, int &end)
{
    double loss = 0;
    // double balanceNodeA = abs(remainCpuCores_A - remainMemorySize_A) / (remainCpuCores_A + remainMemorySize_A);//除号是为了归一化
    // double balanceNodeB = abs(remainCpuCores_B - remainMemorySize_B) / (remainCpuCores_B + remainMemorySize_B);
    // double balanceNodeAB = abs(remainCpuCores_A + remainCpuCores_B - remainMemorySize_A - remainMemorySize_B) /
    //                        (remainCpuCores_A + remainCpuCores_B + remainMemorySize_A + remainMemorySize_B);
    double remainCpuAndMemory = remainCpuCores_A + remainCpuCores_B + remainMemorySize_A + remainMemorySize_B;
    //double svrTotal=
    if (onNode == AB)
    {
        double balanceNodeA = remainCpuCores_A + remainCpuCores_B;
        double balanceNodeB = remainMemorySize_A + remainMemorySize_B;
        double svrRemainRate = abs(balanceNodeB - balanceNodeA);
        loss += balanceNodeA * balanceNodeA_coef +
                balanceNodeB * balanceNodeB_coef + remainCpuAndMemory * remainCpuAndMemory_coef+ svrRemainRate * svrRemainRate_coef -
                svrRunVmNums * svrRunVmNums_coef - svr.totalRes * totalRes_coef;
 
        if (((remainCpuCores_A < lowCpuFor2Nodes_coef) ||
             (remainMemorySize_A < lowMemoryFor2Nodes_coef) ||
             (remainCpuCores_B < lowCpuFor2Nodes_coef) ||
             (remainMemorySize_B < lowMemoryFor2Nodes_coef)) &&
            remainCpuAndMemory > 100)
            loss = loss + 1e5;
        return loss;
    }
    else if (onNode == A)
    {
        double balanceNodeA = remainCpuCores_A;
        double balanceNodeB = remainMemorySize_A;
        double svrRemainRate = abs(balanceNodeB - balanceNodeA);
        loss += balanceNodeA * balanceNodeA_coef +
                balanceNodeB * balanceNodeB_coef + remainCpuAndMemory * remainCpuAndMemory_coef + svrRemainRate * svrRemainRate_coef -
                svrRunVmNums * svrRunVmNums_coef - svr.totalRes * totalRes_coef;
        if (((remainCpuCores_A < lowCpu_coef) || (remainMemorySize_A < lowMemory_coef)) &&
            (3 * remainCpuCores_A + remainMemorySize_A) > 30)
            loss = loss + 1e5;

        return loss;
    }
    else
    {
        double balanceNodeA = remainCpuCores_B;
        double balanceNodeB = remainMemorySize_B;
        double svrRemainRate = abs(balanceNodeB - balanceNodeA);
        loss += balanceNodeA * balanceNodeA_coef +
                balanceNodeB * balanceNodeB_coef + remainCpuAndMemory * remainCpuAndMemory_coef + svrRemainRate * svrRemainRate_coef -
                svrRunVmNums * svrRunVmNums_coef - svr.totalRes * totalRes_coef;
        if (((remainCpuCores_B < lowCpu_coef) || (remainMemorySize_B < lowMemory_coef)) &&
            (3 * remainCpuCores_B + remainMemorySize_B) > 30)
            loss = loss + 1e5;
        return loss;
    }

    //   balanceNodeAB * balanceNodeAB_coef +
    //   remainCpuAndMemory * remainCpuAndMemory_coef -//剩余的CPU和Memory要尽可能少
    //   svrRunVmNums * svrRunVmNums_coef;//服务器运行的VmNums要尽可能多（注意﹣号）
}

bestSvrForVm getBestSvrForVm(ownSvrInfo &svr, notDeployVmInfo &vm, int &start, int &end)
{
    if (vm.is2Nodes)
    {
        if (svr.remainCpuCores_A >= vm.cpuCores / 2 && svr.remainCpuCores_B >= vm.cpuCores / 2 &&
            svr.remainMemorySize_A >= vm.memorySize / 2 && svr.remainMemorySize_B >= vm.memorySize / 2)
        {
            double remainCpuCores_A = svr.remainCpuCores_A - vm.cpuCores / 2;
            double remainCpuCores_B = svr.remainCpuCores_B - vm.cpuCores / 2;
            double remainMemorySize_A = svr.remainMemorySize_A - vm.memorySize / 2;
            double remainMemorySize_B = svr.remainMemorySize_B - vm.memorySize / 2;
            double loss = computeLossForBestSvrForVm(remainCpuCores_A, remainCpuCores_B,
                                                     remainMemorySize_A, remainMemorySize_B,
                                                     svr.svrRunVms.size(), vm.cpuCores, vm.memorySize, svr, AB, start, end);

            return bestSvrForVm{loss, svr.svrId, AB};
        }
        else
        {
            return bestSvrForVm{INF, -1, AB};
        }
    }

    bestSvrForVm bestSvr = bestSvrForVm{INF, -1, AB}; //A、B结点都去遍历
    if (svr.remainCpuCores_A >= vm.cpuCores && svr.remainMemorySize_A >= vm.memorySize)
    {
        double remainCpuCores_A = svr.remainCpuCores_A - vm.cpuCores;
        double remainCpuCores_B = svr.remainCpuCores_B;
        double remainMemorySize_A = svr.remainMemorySize_A - vm.memorySize;
        double remainMemorySize_B = svr.remainMemorySize_B;

        double loss = computeLossForBestSvrForVm(remainCpuCores_A, remainCpuCores_B,
                                                 remainMemorySize_A, remainMemorySize_B,
                                                 svr.svrRunVms.size(), vm.cpuCores, vm.memorySize, svr, A, start, end);

        if (loss < bestSvr.loss)
        {
            bestSvr = bestSvrForVm{loss, svr.svrId, A};
        }
    }
    if (svr.remainCpuCores_B >= vm.cpuCores && svr.remainMemorySize_B >= vm.memorySize)
    {
        double remainCpuCores_A = svr.remainCpuCores_A;
        double remainCpuCores_B = svr.remainCpuCores_B - vm.cpuCores;
        double remainMemorySize_A = svr.remainMemorySize_A;
        double remainMemorySize_B = svr.remainMemorySize_B - vm.memorySize;
        double loss = computeLossForBestSvrForVm(remainCpuCores_A, remainCpuCores_B,
                                                 remainMemorySize_A, remainMemorySize_B,
                                                 svr.svrRunVms.size(), vm.cpuCores, vm.memorySize, svr, B, start, end);

        if (loss < bestSvr.loss)
        {
            bestSvr = bestSvrForVm{loss, svr.svrId, B};
        }
    }
    return bestSvr;
}

bool createVM(notDeployVmInfo &createVmInfo, int &start, int &end)
{
    bestSvrForVm bestSvr = bestSvrForVm{INF, -1, AB};
    for (int i = 0; i < svrNum; i++)
    {
        bestSvrForVm _bestSvr = getBestSvrForVm(ownSvr[i], createVmInfo, start, end);
        if (_bestSvr.loss < bestSvr.loss)
        {
            bestSvr = _bestSvr;
        }
    }
    if (bestSvr.svrId == -1)
    {
        return 0;
    }
    selectSvr(ownSvr[bestSvr.svrId], createVmInfo, bestSvr.on);
    ownSvr[bestSvr.svrId].svrRunVms.push_back(createVmInfo.vmId);
    vmNum++;
    return 1;
}

void delVM(string &delVmId)
{
    string t_vmId = delVmId;
    vmRunOnSvrInfo t_vmInfo = vmRunOnSvr[t_vmId];
    int t_serverId = t_vmInfo.svrId;
    ownSvrInfo &server = ownSvr[t_serverId];
    vector<string> &t_svrRunVms = server.svrRunVms;
    t_svrRunVms.erase(remove(t_svrRunVms.begin(), t_svrRunVms.end(), t_vmId), t_svrRunVms.end());
    vmNum--;
    if (t_vmInfo.on == AB)
    {
        int cores = t_vmInfo.vmCores >> 1, memory = t_vmInfo.vmMemory >> 1;
        server.remainCpuCores_A += cores;
        server.remainCpuCores_B += cores;
        server.remainMemorySize_A += memory;
        server.remainMemorySize_B += memory;
    }
    else
    {
        if (t_vmInfo.on == A)
        {
            server.remainCpuCores_A += t_vmInfo.vmCores;
            server.remainMemorySize_A += t_vmInfo.vmMemory;
        }
        else
        {
            server.remainCpuCores_B += t_vmInfo.vmCores;
            server.remainMemorySize_B += t_vmInfo.vmMemory;
        }
    }
}

bool cmpUndeployVms(const notDeployVmInfo &a, const notDeployVmInfo &b)
{
    return a.scores > b.scores;
}
bool cmpMigrationSvr(const int &a, const int &b)
{
    ownSvrInfo svr1 = ownSvr[a];
    ownSvrInfo svr2 = ownSvr[b];
    return svr1.remainCpuCores_A + svr1.remainCpuCores_B + svr1.remainMemorySize_A + svr1.remainMemorySize_B <
           svr2.remainCpuCores_A + svr2.remainCpuCores_B + svr2.remainMemorySize_A + svr2.remainMemorySize_B;
}


bool try_createVM(notDeployVmInfo &createVmInfo, int &start, int &end)
{
    bestSvrForVm bestSvr = bestSvrForVm{INF, -1, AB};
    for (int i = 0; i < svrNum; i++)
    {
        bestSvrForVm _bestSvr = getBestSvrForVm(ownSvr[i], createVmInfo, start, end);
        if (_bestSvr.loss < bestSvr.loss)
        {
            bestSvr = _bestSvr;
        }
    }
    if (bestSvr.svrId == -1)
    {
        return 0;
    }
    return 1;
}

void dealReqSegments(vector<vector<string>> &reqSegments, int &start, int &end)
{
    if (reqSegments.size() == 0)
        return;
    vector<notDeployVmInfo> reqVmInfos;
    int size = 0;
    deployInfosForSegment.clear();
    for (auto reqSegment : reqSegments)
    {
        string reqVmType = reqSegment[1], reqVmId = reqSegment[2];
        int cpuCores = vmInfos[reqVmType].cpuCores;
        int memorySize = vmInfos[reqVmType].memorySize;
        int scores = cpuCores + memorySize;
        notDeployVmInfo t_notDeployVmInfo = notDeployVmInfo{reqVmId, cpuCores, memorySize, scores, vmInfos[reqVmType].is2Nodes, size++};
        reqVmInfos.push_back(t_notDeployVmInfo);
    }
    reqSegments.clear();
    deployInfosForSegment.resize(size);
    sort(reqVmInfos.begin(), reqVmInfos.end(), cmpUndeployVms);
    vector<notDeployVmInfo> canDeployInfos;

    for (auto reqVmInfo : reqVmInfos)
    {
        if (try_createVM(reqVmInfo, start, end) == 0)
        {
            addSvr(reqVmInfo, start, end);
            createVM(reqVmInfo, start, end);
        }
        else
        {
            canDeployInfos.push_back(reqVmInfo);
        }
    }
    for (auto reqVmInfo : canDeployInfos)
    {
        while (createVM(reqVmInfo, start, end) == 0)
        {
            addSvr(reqVmInfo, start, end);
        }
    }

    for (int i = 0; i < size; ++i)
    {
        deployInfos.push_back(deployInfosForSegment[i]);
    }
}

void invadeMigration(vmRunOnSvrInfo &invadeVm, bestSvrForVmToMigration &bestSvrForVmToMigration)
{
    delVM(invadeVm.vmId);
    if (bestSvrForVmToMigration.on == AB)
    {
        ownSvrInfo &svr = ownSvr[bestSvrForVmToMigration.svrId];
        int needCores = invadeVm.vmCores / 2;
        int needMemory = invadeVm.vmMemory / 2;
        svr.remainCpuCores_A -= needCores;
        svr.remainCpuCores_B -= needCores;
        svr.remainMemorySize_A -= needMemory;
        svr.remainMemorySize_B -= needMemory;
        vmRunOnSvr[invadeVm.vmId] = vmRunOnSvrInfo{invadeVm.vmId, svr.svrId, invadeVm.vmCores, invadeVm.vmMemory, AB};
        ownSvr[svr.svrId].svrRunVms.push_back(invadeVm.vmId);
        vmNum++;
        vector<int> migrationInfo;
        migrationInfo.push_back(atoi(invadeVm.vmId.c_str()));
        migrationInfo.push_back(svr.svrId);
        migrationInfo.push_back(AB);
        migrationInfos.push_back(migrationInfo);

        return;
    }
    else if (bestSvrForVmToMigration.on == A)
    {
        ownSvrInfo &svr = ownSvr[bestSvrForVmToMigration.svrId];
        svr.remainCpuCores_A -= invadeVm.vmCores;
        svr.remainMemorySize_A -= invadeVm.vmMemory;
        vmRunOnSvr[invadeVm.vmId] = vmRunOnSvrInfo{invadeVm.vmId, svr.svrId, invadeVm.vmCores, invadeVm.vmMemory, A};
        ownSvr[svr.svrId].svrRunVms.push_back(invadeVm.vmId);
        vmNum++;
        vector<int> migrationInfo;
        migrationInfo.push_back(atoi(invadeVm.vmId.c_str()));
        migrationInfo.push_back(svr.svrId);
        migrationInfo.push_back(A);
        migrationInfos.push_back(migrationInfo);
        return;
    }
    else
    {
        ownSvrInfo &svr = ownSvr[bestSvrForVmToMigration.svrId];
        svr.remainCpuCores_B -= invadeVm.vmCores;
        svr.remainMemorySize_B -= invadeVm.vmMemory;
        vmRunOnSvr[invadeVm.vmId] = vmRunOnSvrInfo{invadeVm.vmId, svr.svrId, invadeVm.vmCores, invadeVm.vmMemory, B};
        ownSvr[svr.svrId].svrRunVms.push_back(invadeVm.vmId);
        vmNum++;
        vector<int> migrationInfo;
        migrationInfo.push_back(atoi(invadeVm.vmId.c_str()));
        migrationInfo.push_back(svr.svrId);
        migrationInfo.push_back(B);
        migrationInfos.push_back(migrationInfo);
        return;
    }
}

bestSvrForVmToMigration getBestSvrForVmToMigration(vmRunOnSvrInfo &vm, ownSvrInfo &svrInvaded, int &start, int &end)
{
    if (vm.on == AB)
    {
        if (svrInvaded.remainCpuCores_A >= vm.vmCores / 2 && svrInvaded.remainCpuCores_B >= vm.vmCores / 2 &&
            svrInvaded.remainMemorySize_A >= vm.vmMemory / 2 && svrInvaded.remainMemorySize_B >= vm.vmMemory / 2)
        {
            double remainCpuCores_A = svrInvaded.remainCpuCores_A - vm.vmCores / 2;
            double remainCpuCores_B = svrInvaded.remainCpuCores_B - vm.vmCores / 2;
            double remainMemorySize_A = svrInvaded.remainMemorySize_A - vm.vmMemory / 2;
            double remainMemorySize_B = svrInvaded.remainMemorySize_B - vm.vmMemory / 2;
            double loss = computeLossForBestSvrForVm(remainCpuCores_A, remainCpuCores_B,
                                                     remainMemorySize_A, remainMemorySize_B, svrInvaded.svrRunVms.size(), vm.vmCores, vm.vmMemory, svrInvaded,
                                                     AB, start, end);
            return bestSvrForVmToMigration{loss, svrInvaded.svrId, AB};
        }
        else
        {
            return bestSvrForVmToMigration{INF, -1, AB};
        }
    }

    bestSvrForVmToMigration bestSvr = bestSvrForVmToMigration{INF, -1, AB}; //A、B结点都去遍历
    if (svrInvaded.remainCpuCores_A >= vm.vmCores && svrInvaded.remainMemorySize_A >= vm.vmMemory)
    {
        double remainCpuCores_A = svrInvaded.remainCpuCores_A - vm.vmCores;
        double remainCpuCores_B = svrInvaded.remainCpuCores_B;
        double remainMemorySize_A = svrInvaded.remainMemorySize_A - vm.vmMemory;
        double remainMemorySize_B = svrInvaded.remainMemorySize_B;

        double loss = computeLossForBestSvrForVm(remainCpuCores_A, remainCpuCores_B,
                                                 remainMemorySize_A, remainMemorySize_B, svrInvaded.svrRunVms.size(),
                                                 vm.vmCores, vm.vmMemory, svrInvaded, A, start, end);

        if (loss < bestSvr.loss)
        {
            bestSvr = bestSvrForVmToMigration{loss, svrInvaded.svrId, A};
        }
    }
    if (svrInvaded.remainCpuCores_B >= vm.vmCores && svrInvaded.remainMemorySize_B >= vm.vmMemory)
    {
        double remainCpuCores_A = svrInvaded.remainCpuCores_A;
        double remainCpuCores_B = svrInvaded.remainCpuCores_B - vm.vmCores;
        double remainMemorySize_A = svrInvaded.remainMemorySize_A;
        double remainMemorySize_B = svrInvaded.remainMemorySize_B - vm.vmMemory;

        double loss = computeLossForBestSvrForVm(remainCpuCores_A, remainCpuCores_B,
                                                 remainMemorySize_A, remainMemorySize_B, svrInvaded.svrRunVms.size(),
                                                 vm.vmCores, vm.vmMemory, svrInvaded, B, start, end);

        if (loss < bestSvr.loss)
        {
            bestSvr = bestSvrForVmToMigration{loss, svrInvaded.svrId, B};
        }
    }
    return bestSvr;
}

bestSvrForVmToMigration getBestSvrForVmToMigration_sameSvr(vmRunOnSvrInfo &vm, ownSvrInfo &svrInvaded, int &start, int &end)
{
    if (vm.on == AB)
    {
        return bestSvrForVmToMigration{INF, -1, AB};
    }

    bestSvrForVmToMigration bestSvr = bestSvrForVmToMigration{INF, -1, AB}; //A、B结点都去遍历
    if (vm.on == B && svrInvaded.remainCpuCores_A >= vm.vmCores && svrInvaded.remainMemorySize_A >= vm.vmMemory)
    {
        double remainCpuCores_A = svrInvaded.remainCpuCores_A - vm.vmCores;
        double remainCpuCores_B = svrInvaded.remainCpuCores_B;
        double remainMemorySize_A = svrInvaded.remainMemorySize_A - vm.vmMemory;
        double remainMemorySize_B = svrInvaded.remainMemorySize_B;

        double loss = computeLossForBestSvrForVm(remainCpuCores_A, remainCpuCores_B,
                                                 remainMemorySize_A, remainMemorySize_B, svrInvaded.svrRunVms.size(),
                                                 vm.vmCores, vm.vmMemory, svrInvaded, A, start, end);

        if (loss < bestSvr.loss)
        {
            bestSvr = bestSvrForVmToMigration{loss, svrInvaded.svrId, A};
        }
    }
    if (vm.on == A && svrInvaded.remainCpuCores_B >= vm.vmCores && svrInvaded.remainMemorySize_B >= vm.vmMemory)
    {
        double remainCpuCores_A = svrInvaded.remainCpuCores_A;
        double remainCpuCores_B = svrInvaded.remainCpuCores_B - vm.vmCores;
        double remainMemorySize_A = svrInvaded.remainMemorySize_A;
        double remainMemorySize_B = svrInvaded.remainMemorySize_B - vm.vmMemory;

        double loss = computeLossForBestSvrForVm(remainCpuCores_A, remainCpuCores_B,
                                                 remainMemorySize_A, remainMemorySize_B, svrInvaded.svrRunVms.size(),
                                                 vm.vmCores, vm.vmMemory, svrInvaded, B, start, end);

        if (loss < bestSvr.loss)
        {
            bestSvr = bestSvrForVmToMigration{loss, svrInvaded.svrId, B};
        }
    }
    return bestSvr;
}

double computeUseRateForBestMigration(ownSvrInfo &svr)
{
    // double remain = svr.remainCpuCores_A + svr.remainMemorySize_A +
    //                 svr.remainCpuCores_B + svr.remainMemorySize_B;

    // return 1.0 - remain / svr.totalRes;
    double remain = 3 * (svr.remainCpuCores_A + svr.remainCpuCores_B) +
                    svr.remainMemorySize_A + svr.remainMemorySize_B;

    return remain;
}

void dealMigration(int &start, int &end)
{
    int migrationTimes = vmNum * 3 / 100;
    if (migrationTimes <= 0)
        return;
    vector<int> svrIdForNeedMigration;
    for (int i = 0; i < svrNum; i++)
    {
        double useRate = computeUseRateForBestMigration(ownSvr[i]); //计算使用率 TODO，还可考虑其他方法
        if (useRate > 20)                                           //19 20 22
        {
            svrIdForNeedMigration.push_back(i);
        }
    }
    sort(svrIdForNeedMigration.begin(), svrIdForNeedMigration.end(), cmpMigrationSvr);
    int svrIdForNeedMigrationSize = svrIdForNeedMigration.size();
    for (int i = svrIdForNeedMigrationSize - 1; i >= 0; i--)
    {
        int svrIdInvade = svrIdForNeedMigration[i];
        int vmSize = ownSvr[svrIdInvade].svrRunVms.size();
        for (int k = 0; k < vmSize; k++)
        {
            vmRunOnSvrInfo vmInvade = vmRunOnSvr[ownSvr[svrIdInvade].svrRunVms[k]];
            ownSvrInfo svr = ownSvr[svrIdInvade];
            double loss = computeLossForBestSvrForVm(svr.remainCpuCores_A, svr.remainCpuCores_B,
                                                     svr.remainMemorySize_A, svr.remainMemorySize_B, svr.svrRunVms.size(),
                                                     vmInvade.vmCores, vmInvade.vmMemory, svr, AB, start, end);
            bestSvrForVmToMigration bestInvadedSvr = bestSvrForVmToMigration{loss, -1, AB};
            for (int j = 0; j < svrIdForNeedMigrationSize; j++)
            {
                int svrIdInvaded = svrIdForNeedMigration[j];
                bestSvrForVmToMigration _bestInvadedSvr;
                if (svrIdInvade == svrIdInvaded)
                {
                    _bestInvadedSvr = getBestSvrForVmToMigration_sameSvr(vmInvade, ownSvr[svrIdInvaded], start, end);
                }
                else
                {
                    _bestInvadedSvr = getBestSvrForVmToMigration(vmInvade, ownSvr[svrIdInvaded], start, end);
                }

                if (_bestInvadedSvr.loss < bestInvadedSvr.loss)
                {
                    bestInvadedSvr = _bestInvadedSvr;
                }
            }
            if (bestInvadedSvr.svrId == -1)
            {
                continue;
            }
            //迁移
            invadeMigration(vmInvade, bestInvadedSvr);
            vmSize--; //因为服务器上的虚拟机少了一个
            migrationNum++;
            migrationTimes--;
            if (migrationTimes <= 0)
                return;
        }
    }
}

void dealDeploy(int &day, int &start, int &end)
{
    vector<vector<string>> reqSegments; //一连串的add请求
    for (auto req : reqInfos[day])
    {
        int opType = req.size() == 3 ? 1 : 0;
        if (opType)
        {
            reqSegments.push_back(req);
        }
        else
        {
            dealReqSegments(reqSegments, start, end);
            delVM(req[1]);
        }
    }
    dealReqSegments(reqSegments, start, end);
}

void decision(int day, int start, int end)
{
    lowCpu_coef = INF;
    lowMemory_coef = INF;
    lowCpuFor2Nodes_coef = INF;
    lowMemoryFor2Nodes_coef = INF;
    for (auto req : reqInfos[day])
    {
        int opType = req.size() == 3 ? 1 : 0;
        if (opType)
        {
            if (vmInfos[req[1]].is2Nodes)
            {
                lowCpuFor2Nodes_coef = min(vmInfos[req[1]].cpuCores / 2.0, lowCpuFor2Nodes_coef);
                lowMemoryFor2Nodes_coef = min(vmInfos[req[1]].memorySize / 2.0, lowMemoryFor2Nodes_coef);
            }
            else
            {
                lowCpu_coef = min(vmInfos[req[1]].cpuCores * 1.0, lowCpu_coef);
                lowMemory_coef = min(vmInfos[req[1]].memorySize * 1.0, lowMemory_coef);
            }
        }
    }

    migrationInfos.clear(); //今天的迁移情况
    dealMigration(start, end);
    SvrBuyToday.clear(); //今天够买的服务器
    deployInfos.clear(); //今天的部署情况

    dealDeploy(day, start, end);


    pushPurchaseInfos(day);
    pushMigrationInfos(day);
    pushDeployInfos(day);
}

//-----------------------------计算总成本成本--------------------------------
void todaySvrPowerCost()
{
    for (int i = 0; i < svrNum; i++)
    {
        if (ownSvr[i].svrRunVms.size() > 0)
        {
            POWERCOST += ownSvr[i].powerCost;
        }
    }
}

void getDayReqs(int &d)
{
    string repOp, reqVmType, reqId;
    int dayReqNum = 0;
    scanf("%d", &dayReqNum);
    for (int i = 0; i < dayReqNum; ++i)
    {
        cin >> repOp;
        if (repOp[1] == 'a')
        {
            cin >> reqVmType >> reqId;
            genReq(repOp, reqVmType, reqId, d);
        }
        else
        {
            cin >> reqId;
            genReq(repOp, reqId, d);
        }
    }
}

void changeSlideWindowInfo_Add(int addDay)
{
    for (auto req : reqInfos[addDay])
    {
        int opType = req.size() == 3 ? 1 : 0;
        if (opType)
        {
            string vmType = req[1];
            string vmId = req[2];
            vmId2Type[vmId] = vmType;
            slideWindowInfo.totalVmsNum++;
            slideWindowInfo.needCpu += vmInfos[vmType].cpuCores;
            slideWindowInfo.needMemory += vmInfos[vmType].memorySize;
            if (vmInfos[vmType].is2Nodes)
                slideWindowInfo.twoNodesNum++;
        }
        else
        {
            string vmId = req[1];
            string vmType = vmId2Type[vmId];
            slideWindowInfo.totalVmsNum--;
            slideWindowInfo.needCpu -= vmInfos[vmType].cpuCores;
            slideWindowInfo.needMemory -= vmInfos[vmType].memorySize;
            if (vmInfos[vmType].is2Nodes)
                slideWindowInfo.twoNodesNum--;
        }
    }
}

void changeSlideWindowInfo_Del(int delDay)
{
    for (auto req : reqInfos[delDay])
    {
        int opType = req.size() == 3 ? 1 : 0;
        if (opType)
        {
            string vmType = req[1];
            string vmId = req[2];
            vmId2Type[vmId] = vmType;
            slideWindowInfo.totalVmsNum--;
            slideWindowInfo.needCpu -= vmInfos[vmType].cpuCores;
            slideWindowInfo.needMemory -= vmInfos[vmType].memorySize;
            if (vmInfos[vmType].is2Nodes)
                slideWindowInfo.twoNodesNum--;
        }
        else
        {
            string vmId = req[1];
            string vmType = vmId2Type[vmId];
            slideWindowInfo.totalVmsNum++;
            slideWindowInfo.needCpu += vmInfos[vmType].cpuCores;
            slideWindowInfo.needMemory += vmInfos[vmType].memorySize;
            if (vmInfos[vmType].is2Nodes)
                slideWindowInfo.twoNodesNum++;
        }
    }
}

void changeSlideWindowInfo_Del_Add(int delDay, int addDay)
{
    for (auto req : reqInfos[delDay])
    {
        int opType = req.size() == 3 ? 1 : 0;
        if (opType)
        {
            string vmType = req[1];
            string vmId = req[2];
            vmId2Type[vmId] = vmType;
            slideWindowInfo.totalVmsNum--;
            slideWindowInfo.needCpu -= vmInfos[vmType].cpuCores;
            slideWindowInfo.needMemory -= vmInfos[vmType].memorySize;
            if (vmInfos[vmType].is2Nodes)
                slideWindowInfo.twoNodesNum--;
        }
        else
        {
            string vmId = req[1];
            string vmType = vmId2Type[vmId];
            slideWindowInfo.totalVmsNum++;
            slideWindowInfo.needCpu += vmInfos[vmType].cpuCores;
            slideWindowInfo.needMemory += vmInfos[vmType].memorySize;
            if (vmInfos[vmType].is2Nodes)
                slideWindowInfo.twoNodesNum++;
        }
    }

    for (auto req : reqInfos[addDay])
    {
        int opType = req.size() == 3 ? 1 : 0;
        if (opType)
        {
            string vmType = req[1];
            string vmId = req[2];
            vmId2Type[vmId] = vmType;
            slideWindowInfo.totalVmsNum++;
            slideWindowInfo.needCpu += vmInfos[vmType].cpuCores;
            slideWindowInfo.needMemory += vmInfos[vmType].memorySize;
            if (vmInfos[vmType].is2Nodes)
                slideWindowInfo.twoNodesNum++;
        }
        else
        {
            string vmId = req[1];
            string vmType = vmId2Type[vmId];
            slideWindowInfo.totalVmsNum--;
            slideWindowInfo.needCpu -= vmInfos[vmType].cpuCores;
            slideWindowInfo.needMemory -= vmInfos[vmType].memorySize;
            if (vmInfos[vmType].is2Nodes)
                slideWindowInfo.twoNodesNum--;
        }
    }
}

int main()
{
#ifdef TEST
    int seed = 20;
    srand(seed);
    clock_t start, end;
    int runPercent = 0;
    //const string filePath = "./training-data/training-2.txt";
    freopen(filePath.c_str(), "rb", stdin);
    start = clock();
#endif
    //服务器信息读取
    int svrCount;
    string svrType, svrCpuCores, svrMemorySize, svrCost, svrPowerCost;
    scanf("%d", &svrCount);
    max_seq = svrCount;
    seq2SvrType.resize(max_seq);
    for (int i = 0; i < svrCount; ++i)
    {
        cin >> svrType >> svrCpuCores >> svrMemorySize >> svrCost >> svrPowerCost;
        genSvr(svrType, svrCpuCores, svrMemorySize, svrCost, svrPowerCost, i);
    }
    AvgSvrCost = AvgSvrCost / svrCount;
    AvgPowerCost = AvgPowerCost / svrCount;

    //虚拟机信息读取
    int vmCount = 0;
    cin >> vmCount;
    string vmType, vmCpuCores, vmMemorySize, vmIs2Nodes;
    for (int i = 0; i < vmCount; ++i)
    {
        cin >> vmType >> vmCpuCores >> vmMemorySize >> vmIs2Nodes;
        genVm(vmType, vmCpuCores, vmMemorySize, vmIs2Nodes);
    }

    //请求读取，这次我们使用一次性读取所有天的请求
    int reqDays = 0;
    scanf("%d %d", &reqDays, &K);
    TotalDay = reqDays;
    out.resize(reqDays + 1);
    reqInfos.resize(reqDays + 1);
    for (int d = 1; d <= reqDays; ++d)
    {
        if (d <= K)
        {
            getDayReqs(d);
            changeSlideWindowInfo_Add(d); //add
            // testfun();
        }
        else
        {
            decision(d - K, d - K, d - 1); //TODO

#ifdef TEST
            todaySvrPowerCost();
            int curPercent = (d - K) * 100 / reqDays;
            if (curPercent >= runPercent)
            {
                runPercent = curPercent;
                printf("\rcomplete %3d%%", runPercent);
            }
#endif
#ifdef COMMIT
            vector<string> _out = out[d - K];
            for (auto &o : _out)
            {
                cout << o;
            }
            fflush(stdout);
#endif
            getDayReqs(d);
            changeSlideWindowInfo_Del_Add(d - K, d); //del and add
            // testfun();
        }
    }

    for (int i = reqDays - K + 1; i <= reqDays; i++)
    {
        decision(i, i, reqDays);      //TODO
        changeSlideWindowInfo_Del(i); //del
                                      // testfun();
#ifdef TEST
        todaySvrPowerCost();
        int curPercent = i * 100 / reqDays;
        if (curPercent >= runPercent)
        {
            runPercent = curPercent;
            printf("\rcomplete %3d%%", runPercent);
        }
#endif
#ifdef COMMIT
        vector<string> _out = out[i];
        for (auto &o : _out)
        {
            cout << o;
        }
        fflush(stdout);
#endif
    }
#ifdef TEST
    fclose(stdin);
    end = clock();
    TOTALCOST = SVRCOST + POWERCOST;
    printf("\n\n----------------------------\n");
    printf("Run time: %f s \n", double(end - start) / CLOCKS_PER_SEC);
    printf("Server cost: %lld \n", SVRCOST);
    printf("Power cost: %lld \n", POWERCOST);
    printf("Total cost: %lld \n", TOTALCOST);
    cout << "Training Data: " << filePath << endl;
    printf("svrNum: %d \n", svrNum);
    printf("vmNum: %d \n", vmNum);
    printf("migrationNum: %d", migrationNum);
#endif
    return 0;
}

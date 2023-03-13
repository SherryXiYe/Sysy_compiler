#include "Function.h"
#include "Unit.h"
#include "Type.h"
#include <list>

extern FILE* yyout;
int TreeNode::Num = 0;

Function::Function(Unit *u, SymbolEntry *s)
{
    u->insertFunc(this);
    entry = new BasicBlock(this);   //entry是function的block_list第一个BasicBlock（因为在创建一个BasicBlock的时候，这个BasicBlock会被push_back进f的block_list）
    sym_ptr = s;
    parent = u;
    params = std::vector<SymbolEntry*>();
}

Function::~Function()
{
    auto delete_list = block_list;
    int ii=0;
    for (auto &i : delete_list){
        delete i;
    }
    parent->removeFunc(this);
}

// remove the basicblock bb from its block_list.
void Function::remove(BasicBlock *bb)
{
    block_list.erase(std::find(block_list.begin(), block_list.end(), bb));
}
/*meiyong*/
void Function::dfsClean(){
    std::set<BasicBlock *> v;
    std::list<BasicBlock *> q;
    q.push_back(entry);         
    v.insert(entry);
    while (!q.empty())
    {
        auto bb = q.front();
        q.pop_front();
        for (auto succ = bb->succ_begin(); succ != bb->succ_end(); succ++)
        {
            if (v.find(*succ) == v.end())       //如果没找到（因为如果没找到，find的结果就是最后一个元素的后一个，即v.end）
            {
                v.insert(*succ);
                q.push_back(*succ);
            }
        }
    }
    // if(v.size()!=block_list.size()){
    //     for()
    // }
}

void Function::output() const
{
    FunctionType* funcType = dynamic_cast<FunctionType*>(sym_ptr->getType());
    Type *retType = funcType->getRetType();
    fprintf(yyout, "define %s %s(", retType->toStr().c_str(), sym_ptr->toStr().c_str());
    if(!params.empty()){
        auto paramPtr = params.begin();
        while(paramPtr != params.end()){
            fprintf(yyout, "%s %s", (*paramPtr)->getType()->toStr().c_str(), (*paramPtr)->toStr().c_str());
            paramPtr++;
            if(paramPtr != params.end())
                fprintf(yyout, ", ");
        }
    }
    fprintf(yyout, ") {\n");
    std::set<BasicBlock *> v;
    std::list<BasicBlock *> q;
    q.push_back(entry);         
    v.insert(entry);
    while (!q.empty())
    {
        auto bb = q.front();
        q.pop_front();
        bb->output();
        for (auto succ = bb->succ_begin(); succ != bb->succ_end(); succ++)
        {
            if (v.find(*succ) == v.end())       //如果没找到（因为如果没找到，find的结果就是最后一个元素的后一个，即v.end）
            {
                v.insert(*succ);
                q.push_back(*succ);
            }
        }
    }
    fprintf(yyout, "}\n");
}

void Function::genMachineCode(AsmBuilder* builder) 
{
    auto cur_unit = builder->getUnit();
    auto cur_func = new MachineFunction(cur_unit, this->sym_ptr);
    builder->setFunction(cur_func);
    std::map<BasicBlock*, MachineBlock*> map;
    
    for(auto block : block_list)
    {
        block->genMachineCode(builder);
        map[block] = builder->getBlock();
    }
    // Add pred and succ for every block
    for(auto block : block_list)
    {
        auto mblock = map[block];
        for (auto pred = block->pred_begin(); pred != block->pred_end(); pred++)
            mblock->addPred(map[*pred]);
        for (auto succ = block->succ_begin(); succ != block->succ_end(); succ++)
            mblock->addSucc(map[*succ]);
    }
    cur_unit->InsertFunc(cur_func);

}

void Function::createDfsTree(){
    TreeNode::Num = 0;
    int size = block_list.size();
    dfsTreeRoot = new TreeNode(entry);
    entry->index = 0;
    bool* visited = new bool[size]{};
    dfsTreeMap.resize(size);
    dfsTreeMap[0] = dfsTreeRoot;//建立编号到treeNode的映射
    // 加入visted避免出现环路
    dfs(dfsTreeRoot, visited);
    int deleteNum =0;
    for(int i=0; i<size; i++){
        if(!visited[i]){
            block_list.erase(block_list.begin()+i-deleteNum);
            deleteNum++;
        }
    }
    auto it = dfsTreeMap.end() - deleteNum;
    for(int i = size-deleteNum; i!= size; i++){
        it = dfsTreeMap.erase(it);
    }
    delete[] visited;
}

void Function::dfs(TreeNode* node, bool* visited){
    int idx = std::find(block_list.begin(),block_list.end(), node->block) - block_list.begin();
    visited[idx] = true;
    auto cur_block= node->block;
    for(auto it = cur_block->succ_begin();it!= cur_block->succ_end();it++){
        int cur_idx = std::find(block_list.begin(),block_list.end(), *it) - block_list.begin();
        if(visited[cur_idx])
            continue;
        //如果没访问过child，则进行dfs
        TreeNode* child = new TreeNode(*it);
        (*it)->index = child->No;
        child->parent = node;
        node->addChild(child);
        dfsTreeMap[child->No] = child;
        dfs(child, visited);
    }
}


void Function::computeSemiDom(){
    int size = block_list.size();
    int* parents = new int[size];
    //用于记录临时树各个节点的前驱
    semiDoms.resize(size);
    for (size_t i = 0; i < size; i++){
        semiDoms[i] = i;
        parents[i] = -1;
        //-1代表该节点为根节点
        // std::cout<<dfsTreeMap[i]<<std::endl;
    }

    int num =0;
    for(auto it = dfsTreeMap.rbegin(); (*it)->block != entry; it++){
        auto block = (*it)->block;
        int v = (*it)->No;
        
        for(auto i = block->pred_begin();i != block->pred_end();i++){
            //eval 查找临时树中semiDom值最小的节点
            int z = (*i)->index;
            int p = parents[z];
            while(p != -1 && parents[p] != -1){
                // 不断向上查找
                if(semiDoms[z] > semiDoms[p])
                    z = p;
                p = parents[p];
            }
            if(semiDoms[z] < semiDoms[v])
                semiDoms[v] = semiDoms[z];
        }
        //link
        parents[v] = (*it)->parent->No;
    }
    delete[] parents;
}

void Function::semi_NCA(){
    //https://blog.csdn.net/dashuniuniu/article/details/103462147?spm=1001.2014.3001.5501
    int size = block_list.size();
    immDoms.resize(size);
    domTreeMap.resize(size);
    domTreeRoot = new TreeNode(entry, 0);
    domTreeMap[0] = domTreeRoot;
    immDoms[0] = 0;
    for(int i = 1; i < size; i++){
        int s = semiDoms[i];
        int p = dfsTreeMap[i]->parent->No;
        while (p > s){
            p = immDoms[p];
        }
        immDoms[i] = p;
        TreeNode* parent = domTreeMap[p];
        TreeNode* node = new TreeNode(dfsTreeMap[i]->block, i);
        node->parent = parent;
        parent->addChild(node);
        domTreeMap[i] = node;
    }
}

void Function::computeDomFrontier(){
    //https://blog.csdn.net/Dong_HFUT/article/details/121510224
    for(auto it = block_list.begin(); it != block_list.end(); it++){
        if((*it)->getNumOfPred() > 1){
            for(auto pred = (*it)->pred_begin(); pred != (*it)->pred_end(); pred++){
                int runner = (*pred)->index;
                while(runner != immDoms[(*it)->index]){
                    dfsTreeMap[runner]->block->addDomFrontier((*it));
                    runner = immDoms[runner];
                }
            }
        }
        // (*it)->showDF();
    }
}

void Function::addPhiBlocks(std::set<BasicBlock*> phiBlock){
    std::set_union(phiBlock.begin(), phiBlock.end(), PhiBlocks.begin(), PhiBlocks.end(), std::inserter(PhiBlocks, PhiBlocks.begin()));
}
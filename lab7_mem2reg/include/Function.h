#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <iostream>
#include "BasicBlock.h"
#include "SymbolTable.h"
#include "AsmBuilder.h"

class Unit;
struct TreeNode{
    static int Num;
    int No;
    TreeNode* parent;
    BasicBlock* block;
    std::vector<TreeNode*> children;
    void addChild(TreeNode* child){
        children.push_back(child);
    }
    TreeNode(BasicBlock* block){
        this->block = block;
        No = Num++;
    }
    TreeNode(BasicBlock* block, int no){
        this->block = block;
        No = no;
    }
};

class Function
{
    typedef std::vector<BasicBlock *>::iterator iterator;
    typedef std::vector<BasicBlock *>::reverse_iterator reverse_iterator;

private:
    std::vector<BasicBlock *> block_list;
    SymbolEntry *sym_ptr;       //这个是函数名的symbolEntry
    BasicBlock *entry;
    Unit *parent;
    std::vector<SymbolEntry*> params;
    /*用于mem2reg算法*/
    TreeNode* dfsTreeRoot;  //dfs树根节点
    TreeNode* domTreeRoot;  //支配树根节点
    std::vector<TreeNode*> dfsTreeMap;  //编号到dfs节点的映射
    std::vector<TreeNode*> domTreeMap;  //编号到dom节点的映射
    std::vector<int> semiDoms;  //半支配节点
    std::vector<int> immDoms;  //直接支配节点
    std::set<BasicBlock*> PhiBlocks; //含有PhiNode的块
    /*end*/
public:
    Function(Unit *, SymbolEntry *);
    ~Function();
    void setParams(std::vector<SymbolEntry*> params){this->params = params;}
    void insertBlock(BasicBlock *bb) { block_list.push_back(bb); };
    BasicBlock *getEntry() { return entry; };
    void remove(BasicBlock *bb);
    void output() const;
    std::vector<BasicBlock *> &getBlockList(){return block_list;};
    iterator begin() { return block_list.begin(); };
    iterator end() { return block_list.end(); };
    reverse_iterator rbegin() { return block_list.rbegin(); };
    reverse_iterator rend() { return block_list.rend(); };
    SymbolEntry *getSymPtr() { return sym_ptr; };
    void genMachineCode(AsmBuilder*);
    /*用于mem2reg算法*/
    void dfsClean();
    void createDfsTree();
    void dfs(TreeNode*, bool* );
    void computeSemiDom();
    void semi_NCA();
    void computeDomFrontier();
    std::set<BasicBlock*> getPhiBlocks(){return PhiBlocks;}
    void addPhiBlocks(std::set<BasicBlock*> phiBlock);
    /*end*/
};

#endif

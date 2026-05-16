#include "HCTree.hpp"


static void clearNode(HCNode* cur_node);
static void printTreeHelper(HCNode* node, string prefix, string edge);



//implement destructor
HCTree::~HCTree(){
    clearNode(root);
    root = nullptr;

    for(std::size_t i = 0; i < leaves.size(); ++i){
        leaves[i] = nullptr;
    }   
}

static void clearNode(HCNode* cur_node){
    if(cur_node == nullptr){
        return;
    }
    clearNode(cur_node->c0);
    clearNode(cur_node->c1);
    delete cur_node;
}




void HCTree::build(const vector<int>& freqs){
    priority_queue<HCNode*, vector<HCNode*>, HCNodePtrComp> pq;
    for(std::size_t i = 0; i < freqs.size(); ++i){
        const int freq = freqs[i];
        if(freq == 0) continue;
        HCNode* node = new HCNode(freq,i);
        leaves[i] = node;
        pq.push(node);
    }

    //Base cases if there are 0 or 1 elements
    if(pq.size() == 0) return;
    else if(pq.size() == 1){
        root = pq.top();
        pq.pop();
        return;
    }

    //Main Loop
    while(pq.size() >= 2){
        //Node 1 is smallest Node
        //Node 2 is second smallest Node
        HCNode* node_1 = pq.top();
        pq.pop();
        HCNode* node_2 = pq.top();
        pq.pop();
        int new_freq = node_1->count + node_2->count;
        HCNode* parent = new HCNode(new_freq, node_1->symbol);

        //point parent to kids
        parent->c0 = node_1;
        parent->c1 = node_2;

        //point kids to parent
        node_1->p = parent;
        node_2->p = parent;

        pq.push(parent);

    }

    root = pq.top();
    pq.pop();
    preprocess(root, {});    
}

unsigned char HCTree::decode(FancyInputStream & in) const{
    
    auto isChild = [](HCNode* cur) { return cur->c0 == nullptr && cur->c1 == nullptr; };

    HCNode* cur_node = root;
    while(!isChild(cur_node)){
        int next_bit = in.read_bit();
        if (next_bit == -1) {
            error("Invalid compressed file: unexpected end of Huffman payload");
        }
        if(next_bit == 0){
            cur_node = cur_node->c0;
        }
        else if(next_bit == 1){
            cur_node = cur_node->c1;
        }
    }
    return cur_node->symbol;


    
}

void HCTree::encode(unsigned char symbol, FancyOutputStream & out) const{
    
    for(const auto& bit : codes[symbol]){
        out.write_bit(bit);
    }
    
}




/**
    HELPER METHODS
 */




/*
    Preprocess all codes so we dont havet to traverse tree each time
*/
void HCTree::preprocess(HCNode* cur_node, vector<int> cur_code){
    if(cur_node == nullptr) return;
    if(cur_node->c0) 
    {
        vector<int> copy = cur_code;
        copy.push_back(0);
        preprocess(cur_node->c0, copy);
    }
    if(cur_node->c1)
    {
        vector<int> copy = cur_code;
        copy.push_back(1);
        preprocess(cur_node->c1, copy);
    }
    if(cur_node->c0 == nullptr && cur_node->c1 == nullptr){
        int symbol = cur_node->symbol;
        codes[symbol] = cur_code;
    }
}

/*
    Recursively prints tree
*/
static void printTreeHelper(HCNode* node, string prefix = "", string edge = "") {
    if (node == nullptr) return;    
    printTreeHelper(node->c1, prefix + "        ", "1");

    cout << prefix;

    if (edge != "") {
        cout << edge << "── ";
    }

    //cout << char(node->symbol) << ":" << node->count;
    
    if (node->c1 == nullptr && node->c0 == nullptr) {
        cout << char(node->symbol) << ":" << node->count;
    } 
    else {
        cout << node->count;
    }
    
    cout << endl;
    printTreeHelper(node->c0, prefix + "        ", "0");
}


void HCTree::printCodes(int lo, int hi){

    for(int i = lo; i < hi; ++i){
        cout << "Symbol: " << (char)i << " Code: ";
        for(const auto &x : codes[i]){
            cout << x << " ";
        }
        cout << endl;
    }
}

void HCTree::printTree(){
    printTreeHelper(root);
}
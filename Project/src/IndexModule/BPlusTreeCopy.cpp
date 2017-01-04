#include <string>
#include <vector>
#include <string.h>
#include "../bufmanager/BufPageManager.h"
#include "../fileio/FileManager.h"
#include "../utils/pagedef.h"

#define node_size 3


/***
    since in here we have an already managed system of data with pages and list
we here use this struct as the pointer of each record. So we have an B+ Tree which
seems quite differen from the normal B+ Tree.
   */
struct node_pos {
    int PageNum;
    int PosNum;
};


/***
   We use this struct as the node structure. As we know, only Leaf nodes of the
   B+ Tree link straightly with the record. So we need to have those Leaf node
   marked. As a B+ Tree it should also manage the parent and sibling.
   */
struct page {
    int isLeaf;                             //1
    node_pos self_pos;                      //2
    node_pos child[node_size * 2];          //2 * 2 * node_size
    node_pos parent;                        //2
    node_pos sibling;                       //2
    long long data[node_size * 2 - 1];      //2 * (2 * node_size - 1)
};                                          //8 * node_size + 5

struct search_pos {
    node_pos pos;
    int index;
};

class BPlusTree {
private:
    FileManager* fileM;
    BufPageManager* BufPageM;
    int fileID;
    int fragID;
    node_pos root;
    int node_len;

public:
    BPlusTree() {
        fileM = new FileManager();              // fileManager, only maintain one file each B+ Tree
        BufPageM = new BufPageManager(fileM);   // Buffer
        node_len = 8 * node_size + 5;
    }

    ~BPlusTree() {
        delete fileM;
        delete BufPageM;
    }

    void setFileName(const char * filename) {
        strcpy(this->filename, filename);
    }

    bool createFile() {
        bool createSucc = (fileM->createFile(filename));
        return createSucc;
    }

    bool openFile() {
        bool openSucc = (fileM->openFile(filename, fileID));
        return openSucc;
    }

    bool closeFile() {
        BufPageM.close();
        bool closeSucc = (fileM->closeFile(fileID));
        return closeSucc;
    }

    page createPageNode(int pageID, int positionID) {
        page pageNode;
        for (int i = 0; i < 2 * node_size - 1; i++) {
            pageNode.data[i] = 0;
        }
        for (int i = 0; i < 2 * node_size; i++) {
            pageNode.child[i].PageNum = 0;
            pageNode.child[i].PosNum = 0;
        }
        pageNode.self_pos.PageNum = pageID;
        pageNode.self_pos.PosNum = positionID;
        pageNode.parent.PageNum = pageID;
        pageNode.parent.PosNum = positionID;
        pageNode.sibling.PageNum = pageID;
        pageNode.sibling.PosNum = positionID;
        pageNode.isLeaf = 1;
        return pageNode;
    }

    node_pos getAverage() {
        int index;
        BufType currentBuf;
        node_pos average;
        average.PageNum = 0;
        if (average.PageNum != 0) {
            return average;
        }
        currentBuf = BufPageM->getPage(fileID, 0, index);
        if (currentBuf[0] == 0) {
            currentBuf[0] = 1;
            BufPageM->markDirty(index);
            int nextIndex;
            BufType nextBuf = BufPageM->getPage(fileID, 1, nextIndex);
            nextBuf[0] = node_len + 1;
            BufPageM->markDirty(nextIndex);
            average.PageNum = 1;
            average.PosNum = 1;
        }
        else {
            int nextIndex;
            BufType nextBuf = BufPageM->getPage(fileID, currentBuf[0], nextIndex);
            if((nextBuf[0] + node_len) >= 2048) {
                currentBuf[0]++;
                BufPageM->markDirty(index);
                nextBuf = BufPageM->getPage(fileID, currentBuf[0], nextIndex);
                nextBuf[0] = node_size + 1;
                BufPageM->markDirty(nextIndex);
                average.PageNum = currentBuf[0];
                average.PosNum = 0;
            }
            else {
                average.PosNum = int(nextBuf[0]);
                nextBuf[0] += node_len;
                BufPageM->markDirty(nextIndex);
                average.PageNum = currentBuf[0];
            }
        }
        return average;
    }

    page getNode(int pageID, int positionID) {
        page node;
        int index;
        BufType currentBuf = BufPageM->getPage(fileID, pageID, index);
        int currentPos = position;
        for(int i = 0; i < 2 * node_size - 1; i++) {
            long long* data = (long long*) (&currentBuf[currentPos]);
            node.data[i] = *data;
            currentPos += 2;
        }
        for(int i = 0; i < 2 * node_size; i++) {
            node.child[i].PageNum = int (currentBuf[currentPos++]);
            node.child[i].PosNum = int (currentBuf[currentPos++]);
        }
        node.self_pos.PageNum = int (currentBuf[currentPos++]);
        node.self_pos.PosNum = int (currentBuf[currentPos++]);
        node.parent.PageNum = int (currentBuf[currentPos++]);
        node.parent.PosNum = int (currentBuf[currentPos++]);
        node.isLeaf = int (currentBuf[currentPos++]);
        node.sibling.PageNum = int (currentBuf[currentPos++]);
        node.sibling.PosNum = int (currentBuf[currentPos++]);
        return node;
    }

    void writeBackNode(page node) {
        int index;
        BufType currentBuf = BufPageM->getPage(fileID, node.self_pos.PageNum, index);
        int currentPos;
        currentPos = node.self_pos.PosNum;
        for(int i = 0; i < 2 * node_size - 1; i++) {
            unsigned int* data = (unsigned int*)(&(node.data[i]));
            currentBuf[currentPos++] = data[0];
            currentBuf[currentPos++] = data[1];
        }
        for (int i = 0; i < 2 * node_size; i++) {
            currentBuf[currentPos++] = (unsigned int)(node.child[i].PageNum);
            currentBuf[currentPos++] = (unsigned int)(node.child[i].PosNum);
        }
        currentBuf[currentPos++] = (unsigned int)(node.self_pos.PageNum);
        currentBuf[currentPos++] = (unsigned int)(node.self_pos.PosNum);
        currentBuf[currentPos++] = (unsigned int)(node.parent.PageNum);
        currentBuf[currentPos++] = (unsigned int)(node.parent.PosNum);
        currentBuf[currentPos++] = (unsigned int)(node.isLeaf);
        currentBuf[currentPos++] = (unsigned int)(node.sibling.PageNum);
        currentBuf[currentPos++] = (unsigned int)(node.sibling.PosNum);
        BufPageM->markDirty(index);
    }

    int pageSize(node_pos pos) {
        if(pos.PageNum == 0 && pos.PosNum == 0) {
            return 0;
        }
        page node = getNode(pos.PageNum, pos.PosNum);
        int result = 0;
        for(int i = 0; i < 2 * node_size - 1; i++) {
            if(node.data[i] != 0) {
                result++;
            }
        }
        return result;
    }

    int maxIndex(node_pos pos) {
        if(pos.PageNum == 0 && pos.PosNum == 0) {
            return 0;
        }
        page.node = getNode(pos.PageNum, pos.PosNum);
        for (int i = 0; i < 2 * node_size - 1; i++) {
            if(node.data[i] == 0) {
                return (i - 1);
            }
        }
        return 2 * node_size - 2;
    }

    void shiftLeft(page node, int start, int offset) {
        int current = start;
        int max = maxIndex(node.self_pos);
        if(max == 0) {
            max = 1;
        }
        for(; current < max; current++) {
            node.data[current] = node.data[current + offset];
            node.data[current + offset] = 0;
        }
        current = start;
        for(; current < max + 1; current++) {
            node.child[current] = node.child[current + offset];
            node.child[current + offset].PageNum = 0;
            node.child[current + offset].PosNum = 0;
        }
        writeBackNode(node);
    }

    void shiftRight(page node, int start, int offset) {
        int max = maxIndex(node.self_pos);
        int index = max;
        for(; index >= start; index--) {
            node.data[index + offset] = node.data[index];
            node.data[index] = 0;
        }
        index = max + 1;
        for(; index >= start; index--) {
            node.child[index + offset] = node.child[index];
            node.child[index].PageNum = 0;
            node.child[index].PosNum = 0;
        }
        writeBackNode(node);
    }

    page splitPage(page node, long long val) {
        long long middle = node.data[data_size - 1];
        node_pos rightPos = getAverage();
        page rightNode = createPageNode(rightPos.PageNum, rightPos.PosNum)；
        int index = node_size;
        int i = 0;
        for (; index < 2 * node_pos - 1; index++) {
            rightNode.data[i] = node.data[index];
            node.data[index] = 0;
            rightNode.child[i] = node.child[index];
            node.child[index].PageNum = 0;
            node.child[index].PosNum = 0;
            if(node.isLeaf == 0 && (rightNode.child[i].PageNum != 0) && (rightNode.child[i].PosNum != 0)) {
                page rightChild = getNode(rightNode.child[i].PageNum, rightNode.child[i].PosNum);
                rightChild.parent = rightNode.self_pos;
                writeBackNode(rightChild);
            }
            i++;
        }
        rightNode.child[data_size - 1] = node.child[2 * node_size - 1];
        if(node.isLeaf == 0 && rightNode.child[node_size - 1].PageNum != 0 && rightNode.child[node_size - 1].PosNum != 0) {
            page rightChild = getNode(rightNode.child[node_size - 1].PageNum, rightNode.child[node_size - 1].PosNum);
            rightChild.parent = rightNode.self_pos;
            writeBackNode(rightNode);
        }
        node.child[2 * node_size - 1].PageNum = 0;
        node.child[2 * node_size - 1].PosNum = 0;
        rightNode.sibling = node.sibling;
        if(node.isLeaf) {
            node.sibling = rightNode.self_pos;
        }
        rightNode.isLeaf = node.isLeaf;
        int isNew = 0;
        if(node.self_pos.PageNum == 0 && node.self_pos.PosNum == 1) {
            page rootHere = createPageNode(0, 1);
            node.parent = rootHere.self_pos;
            node_pos newPos = getAverage();
            while (newPos.PageNum == rightNode.self_pos.PageNum && newPos.PosNum == rightNode.self_pos.PosNum) {
                newPos = getAverage();
            }
            node.self_pos = newPos;
            if(node.isLeaf == 0) {
                for(int i = 0; i < 2 * node_size - 1; i++) {
                    if(node.child[i].PageNum == 0 && node.child[i].PosNum == 0)
                        break;
                    page child = getNode(node.child[i].PageNum, node.child[i].PosNum);
                    child.parent = newPos;
                    writeBackNode(child);
                }
            }
            isNew = 1;
            rootHere.isLeaf = 0;
            writeBackNode(rootHere);
        }
        page parent = getNode(node.parent.PageNum, node.parent.PosNum);
        int first = 0;
        if(isNew) {
            parent.child[0] = node.self_pos;
        }
        for(; first < 2 * node_size; first++) {
            if(parent.child[first].PageNum == node.self_pos.PageNum && parent.child[first].PosNum == node.self_pos.PosNum) {
                break;
            }
            if(parent.data[first] > middle || parent.data[first] == 0) {
                break;
            }
        }
        index = 2 * node_size - 3;
        for(;index >= first; index--) {
            parent.data[index + 1] = parent.data[index];
            parent.child[index + 2] = parent.child[index + 1];
        }
        if(parent.data[first] == 0 && isNew == 0) {
            if(parent.child[first].PageNum == node.self_pos.PageNum && parent.child[first].PosNum == node.self_pos.PosNum) {
                parent.data[first] = middle;
                parent.child[first + 1] = right.self_pos;
            }
            else {
                parent.child[first + 1] = parent.child[first];
                parent.data[first] = middle;
                parent.child[first] = rightNode.self_pos;
            }
        }
        else {
            parent.data[first] = middle;
            parent.child[first + 1] = rightNode.self_pos;
        }
        if(isNew) {
            parent.child[0] = node.self_pos;
        }
        else {
            if(first + 2 <= 2 * data_size - 1 && !(parent.child[first + 2].PageNum == 0 && parent.child[first + 2].PosNum == 0) && (rightNode.sibling.PageNum == 0 && rightNode.sibling.PosNum == 0)) {
                rightNode.sibling = parent.child[first + 2];
            }
        }
        right.parent = parent.self_pos;

        page result = node;
        if(val >= middle) {
            result = right;
        }

        writeBackNode(node);
        writeBackNode(parent);
        writeBackNode(rightNode);

        return result;
    }

    void insertData(page node, long long val; node_pos source) {
        if(pageSize(node.self_pos) == 2 * node_size - 1) {
            node = splitPage(node, val);
        }
        int index = 0;
        for(;index < 2 * node_size - 1; index++) {
            if(node.data[index] == 0 || node.data[index] > val) {
                break;
            }
        }
        if(node.isLeaf == 0) {
            if(!(node.child[index].PageNum == 0 && node.child[index].PosNum == 0)) {
                insertData(getNode(node.child[index].PageNum, node.child[index].PosNum), val, source);
                return;
            }
            else {
                node.data[index - 1] = val;
                writeBackNode(node);
                insertData(getNode(node.child[index - 1].PageNum, node.child[index - 1].PosNum), val, source);
                return;
            }
        }
        if(pageSize(node.self_pos) > 0) {
            shiftRight(node, index, 1);
        }

        node = getNode(node.self_pos.PageNum, node.self_pos.PosNum);
        node.data[index] = val;
        node.child[index] = source;

        writeBackNode(node);
    }

    void addNode(long long val, node_pos pos) {
        insertData(getNode(0, 1), val, pos);
    }

    search_pos keySearchMax(int pageID, int positionID, long long key) {
        node_pos pos;
        pos.PageNum = pageID;
        pos.PosNum = positionID;
        int index = maxIndex(pos);
        int i;
        page current = getNode(pageID, positionID);
        for(i = 0; i < index; i++) {
            if(current.data[i] > key) {
                break;
            }
        }
        search_pos goalPos;
        goalPos.pos.PageNum = 0;
        goalPos.pos.PosNum = 0;
        goalPos.index = 0;
        if(current.isLeaf == 1) {
            if(i > index && current.sibling.PageNum == 0 && current.sibling.PosNum == 0) {
                return goalPos;
            }
            else {
                if(i > index) {
                    goalPos.pos = current.sibling;
                    goalPos.index = 0;
                    page sibling = getNode(current.sibling.PageNum, current.sibling.PosNum);
                    while (sibling.child[0].PageNum == 0 && sibling.child[0].PosNum == 0) {
                        sibling = getNode(sibling.sibling.PageNum, sibling.sibling.PosNum);
                    }
                    goalPos.pos = sibling.self_pos;
                    return goalPos;
                }
                else {
                    goalPos.pos = current.self_pos;
                    goalPos.index = i;
                    return goalPos;
                }
            }
        }
        else {
            return keySearchMax(now.child[i].PageNum, now.child[i].PosNum, key);
        }
    }

    search_pos keySearchMin(int pageID, int positionID, long long key) {
        node_pos pos;
        pos.PageNum = pageID;
        pos.posNum = position;
        int index = maxIndex(pos);
        int i;
        page current = getNode(pageID, positionID);
        for(i = 0; i <= index; i++) {
            if(current.data[i] >= key) {
                break;
            }
        }
        search_pos goalPos;
        goalPos.pos.PageNum = 0;
        goalPos.pos.PosNum = 0;
        goalPos.index = 0;
        if(current.isLeaf == 1) {
            for(i = index; i >= 0; i--) {
                if(current.data[i] < key) {
                    break;
                }
            }
            i++;
            if(current.child[i].PageNum == 0 && current.child[i].PosNum == 0) {
                goalPos.pos = current.sibling;
                goalPos.index = 0;
                page sibling = getNode(current.sibling.PageNum, current.sibling.PosNum);
                while (sibling.child[0].PageNum == 0 && sibling.child[0].PosNum == 0) {
                    sibling = getNode(sibling.sibling.PageNum, sibling.sibling.PosNum);
                }
                goalPos.pos = sibling.self_pos;
            }
            else {
                goalPos.pos = current.self_pos;
                goalPos.index = i;
            }
            return goalPos;
        }
        else {
            return keySearchMin(current.child[i].PageNum, current.child[i].PosNum, key);
        }
    }

    void deleteKey(search_pos pos) {
        if(pos.pos.PageNum == 0 && pos.pos.PosNum == 0) {
            return;
        }
        page node = getNode(pos.pos.PageNum, pos.pos.PosNum);
        for(int i = pos.index; i < 2 * node_size - 2; i++) {
            node.data[i] = node.data[i + 1];
            node.child[i] = node.child[i + 1];
        }
        node.data[2 * node_size - 2] = 0;
        node.child[2 * node_size - 2].PageNum = 0;
        node.child[2 * node_size - 2].PosNum = 0;
        node.child[2 * node_size - 1].PageNum = 0;
        node.child[2 * node_size - 1].PosNum = 0;
        writeBackNode(node);
    }

    search_pos search(search_pos first, search_pos second, int pageNum, int posNum) {
        page temp = getNode(first.pos.PageNum, first.pos.PosNum);
        search_pos goalPos;
        goalPos.pos.PageNum = 0;
        goalPos.pos.PosNum = 0;
        int index = first.index;
        while(1) {
            for(; index < 2 * data_size - 1; index++) {
                if(temp.data[index] == 0) {
                    break;
                }
                if(temp.child[index].PageNum == pageNum && temp.child[index].PosNum == posNum) {
                    goalPos.pos = temp.self_pos;
                    goalPos.index = index;
                    break;
                }
            }
            if((temp.sibling.PageNum == 0 && temp.sibling.PosNum == 0) ||
               (temp.self_pos.PageNum = second.pos.PageNum && temp.self_pos.PosNum == second.pos.PosNum) ||
               (goalPos.pos.PageNum != 0 && goalPos.pos.PosNum != 0)) {
                   break;
               }
               else {
                   temp = getNode(temp.sibling.PageNum, temp.sibling.PosNum);
               }
               index = 0;
        }
        return goalPos;
    }

    void initBPlusTree() {
        int index;
        root.PageNum = 0;
        root.PosNum = 1;
        page rootHere = createPageNode(root.PageNum, root.PosNum);
        writeBackNode(rootHere);
        BufType currentBuf = BufPageM->getPage(fileID, 0, index);
        currentBuf[0] = 0;
        BufPageM->markDirty(index);
    }
};

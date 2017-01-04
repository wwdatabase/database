#ifndef WF_BPLUS_TREE
#define WF_BPLUS_TREE

#include "../bufmanager/BufPageManager.h"
#include "../fileio/FileManager.h"
#include "../utils/pagedef.h"
#include <cstring>
#include <string>

#define DU 3

struct node_position{
    int pageNum;
    int posNum;
};

typedef struct _page{
    long long data[DU*2-1];//(DU*2-1)*2
    node_position self_pos;//2
    node_position child[2*DU];//4*DU
    node_position parent;//2
    int isLeaf;//1
    node_position sibling;//2
} page;//8*DU+5

struct search_pos{
    node_position pos;
    int index;
};

class BPlusTree{
private:
    FileManager* fm;
    BufPageManager* bpm;
    int fileID;
    int fragID;
    node_position root;
    int node_length;
public:
    char filename[1024];

public:
    BPlusTree()
    {
        fm = new FileManager();
        bpm = new BufPageManager(fm);
        node_length = 8*DU + 5;
    }
    ~BPlusTree()
    {
        delete bpm;
        delete fm;
    }
    void setFilename(const char* filename)
    {
        strcpy(this->filename, filename);
    }
    bool createFile()
    {
        bool flag1 = (fm->createFile(filename));
        return flag1;
    }
    bool openFile()
    {
        bool flag1 = (fm->openFile(filename, fileID));
        return flag1;
    }
    int closeFile()
    {
        bpm->close();
        bool flag1 = (fm->closeFile(fileID));
        return flag1;
    }
    void initBPlusTree()
    {
        root.pageNum = 0;
        root.posNum = 1;
        page node_root = createNode(0, 1);
        writeBackNode(node_root);
        int index;
        BufType b = bpm->getPage(fileID, 0, index);
        b[0] = 0;
        bpm->markDirty(index);
    }
    page createNode(int pageID, int position)
    {
        page node;
        for(int i=0;i<2*DU-1;i++)
            node.data[i] = 0;
        for(int i=0;i<2*DU;i++)
        {
            node.child[i].pageNum = 0;
            node.child[i].posNum = 0;
        }
        node.self_pos.pageNum = pageID;
        node.self_pos.posNum = position;
        node.parent.posNum = 0;
        node.parent.pageNum = 0;
        node.isLeaf = 1;
        node.sibling.pageNum = 0;
        node.sibling.posNum = 0;
        return node;
    }
    node_position getAvl()
    {
        node_position avl;
        avl.pageNum = 0;
        int index;
        BufType b;
        if(avl.pageNum != 0)
            return avl;

        b = bpm->getPage(fileID, 0, index);
        if(b[0] == 0)
        {
            b[0] = 1;
            bpm->markDirty(index);
            int ind;
            BufType bb = bpm->getPage(fileID, 1, ind);
            bb[0] = 1 + node_length;
            bpm->markDirty(ind);
            avl.pageNum = 1;
            avl.posNum = 1;
        }
        else
        {
            int ind;
            BufType bb = bpm->getPage(fileID, b[0], ind);
            if(bb[0] + node_length >=2048)
            {
                b[0]++;
                bpm->markDirty(index);
                bb = bpm->getPage(fileID, b[0], ind);
                bb[0] = 1 + node_length;
                bpm->markDirty(ind);
                avl.pageNum = b[0];
                avl.posNum = 1;
            }
            else
            {
                avl.posNum = int(bb[0]);
                bb[0] = bb[0] + node_length;
                bpm->markDirty(ind);
                avl.pageNum = b[0];
            }
        }
        return avl;
    }
    page getNode(int pageID, int position)
    {
        page node;
        int index;
        BufType b = bpm->getPage(fileID, pageID, index);
        int pos = position;
        for(int i=0;i<2*DU-1;i++)
        {
            long long* data = (long long*)(&b[pos]);
            node.data[i] = *data;
            pos+=2;
        }
        for(int i=0;i<2*DU;i++)
        {
            node.child[i].pageNum = int(b[pos++]);
            node.child[i].posNum = int(b[pos++]);
        }
        node.self_pos.pageNum = int(b[pos++]);
        node.self_pos.posNum = int(b[pos++]);
        node.parent.pageNum = int(b[pos++]);
        node.parent.posNum = int(b[pos++]);
        node.isLeaf = int(b[pos++]);
        node.sibling.pageNum = int(b[pos++]);
        node.sibling.posNum = int(b[pos++]);
        return node;
    }
    void writeBackNode(page node)
    {
        int index;
        BufType b = bpm->getPage(fileID, node.self_pos.pageNum, index);
        int pos = node.self_pos.posNum;
        for(int i=0;i<2*DU-1;i++)
        {
            unsigned int *data = (unsigned int*)(&(node.data[i]));
            b[pos++] = data[0];
            b[pos++] = data[1];
        }
        for(int i=0;i<2*DU;i++)
        {
            b[pos++] = (unsigned int)(node.child[i].pageNum);
            b[pos++] = (unsigned int)(node.child[i].posNum);
        }
        b[pos++] = (unsigned int)(node.self_pos.pageNum);
        b[pos++] = (unsigned int)(node.self_pos.posNum);
        b[pos++] = (unsigned int)(node.parent.pageNum);
        b[pos++] = (unsigned int)(node.parent.posNum);
        b[pos++] = (unsigned int)(node.isLeaf);
        b[pos++] = (unsigned int)(node.sibling.pageNum);
        b[pos++] = (unsigned int)(node.sibling.posNum);
        bpm->markDirty(index);
    }
    
    
    int pageSize(node_position pos)
    {
        if(pos.pageNum == 0 && pos.posNum == 0)
            return 0;
        page node = getNode(pos.pageNum, pos.posNum);
        int result = 0;
        for(int i=0;i<DU*2-1;i++)
            if(node.data[i]!=0)
                result++;
        return result;
    }
    
    int maxIndex(node_position pos)
    {
        if(pos.pageNum == 0 && pos.posNum == 0)
            return 0;
        page node = getNode(pos.pageNum, pos.posNum);
        for(int i=0;i<DU*2-1;i++)
            if(node.data[i] == 0)
                return i-1;
        return 2*DU-2;
    }
    
    void shiftLeft(page node, int start, int offset)
    {
        int tmp=start;
        int max=maxIndex(node.self_pos);
        if(max==0)
            max=1;
        for(;tmp<max;tmp++){
            node.data[tmp]=node.data[tmp+offset];
            node.data[tmp+offset]=0;
        }
        tmp=start;
        for(;tmp<max+1;tmp++){
            node.child[tmp]=node.child[tmp+offset];
            node.child[tmp+offset].pageNum=0;
            node.child[tmp+offset].posNum=0;
        }
        writeBackNode(node);
    }
    
    
    void shiftRight(page node,int start,int offset)
    {
        int max=maxIndex(node.self_pos);
        int index=max;
        for(;index>=start;index--)
        {
            node.data[index+offset]=node.data[index];
            node.data[index]=0;
        }
        index=max+1;
        for(;index>=start;index--)
        {
            node.child[index+offset]=node.child[index];
            node.child[index].pageNum = 0;
            node.child[index].posNum = 0;
        }
        writeBackNode(node);
    }
    
    page splitPage(page node, long long val)
    {
        long long mid=node.data[DU-1];
        node_position ri = getAvl();
        page right = createNode(ri.pageNum, ri.posNum);
        int index = DU;
        int i=0;
        for(;index<DU*2-1;index++)
        {
            right.data[i]=node.data[index];
            node.data[index]=0;
            right.child[i]=node.child[index];
            node.child[index].pageNum=0;
            node.child[index].posNum=0;
            if( node.isLeaf == 0 && (right.child[i].pageNum != 0) && (right.child[i].posNum != 0))
            {
                page ri_child = getNode(right.child[i].pageNum, right.child[i].posNum);
                ri_child.parent=right.self_pos;
                writeBackNode(ri_child);
            }
            i++;
        }
        
        right.child[DU-1]=node.child[2*DU-1];
        if(node.isLeaf == 0 && right.child[DU-1].pageNum != 0 && right.child[DU-1].posNum != 0)
        {
            page ri_child = getNode(right.child[DU-1].pageNum, right.child[DU-1].posNum);
            ri_child.parent=right.self_pos;
            writeBackNode(ri_child);
        }
        node.child[2*DU-1].pageNum = 0;
        node.child[2*DU-1].posNum = 0;
        right.sibling = node.sibling;
        if(node.isLeaf)
            node.sibling=right.self_pos;
        
        right.isLeaf=node.isLeaf;
        
        int isNew=0;
        if(node.self_pos.pageNum == 0 && node.self_pos.posNum == 1)
        {
            page root_node=createNode(0, 1);
            node.parent = root_node.self_pos;
            node_position newPos = getAvl();
            while(newPos.pageNum == right.self_pos.pageNum && newPos.posNum == right.self_pos.posNum)
                newPos = getAvl();
            node.self_pos = newPos;
            if(node.isLeaf == 0)
            {
                for(int i=0;i<2*DU-1;i++)
                {
                    if(node.child[i].pageNum == 0 && node.child[i].posNum == 0) break;
                    page child = getNode(node.child[i].pageNum, node.child[i].posNum);
                    child.parent = newPos;
                    writeBackNode(child);
                }
            }
            isNew=1;
            root_node.isLeaf=0;
            writeBackNode(root_node);
        }
        
        page parent = getNode(node.parent.pageNum, node.parent.posNum);
        
        int first=0;

        if(isNew)
        {
            parent.child[0]=node.self_pos;
        }
        
        for(;first<2*DU-1;first++)
        {
            if(parent.child[first].pageNum == node.self_pos.pageNum && parent.child[first].posNum == node.self_pos.posNum)
                break;
            if(parent.data[first]>mid||parent.data[first]==0)
                break;
        }
        
        
        
        index=2*DU-3;
        for(;index>=first;index--)
        {
            parent.data[index+1]=parent.data[index];
            parent.child[index+2]=parent.child[index+1];
        }
        if(parent.data[first]==0 && isNew==0 )
        {
            if(parent.child[first].pageNum == node.self_pos.pageNum && parent.child[first].posNum == node.self_pos.posNum)
            {
                parent.data[first]=mid;
                parent.child[first+1]=right.self_pos;
            }
            else
            {
                parent.child[first+1] = parent.child[first];
                parent.data[first]=mid;
                parent.child[first]=right.self_pos;
            }
        }
        else
        {
            parent.data[first]=mid;
            parent.child[first+1]=right.self_pos;
        }
        if(isNew)
        {
            parent.child[0]=node.self_pos;
        }
        else
        {
            if(first+2<=2*DU-1 && !(parent.child[first+2].pageNum == 0 && parent.child[first+2].posNum==0) && (right.sibling.pageNum == 0 && right.sibling.posNum == 0))
            {
                right.sibling = parent.child[first+2];
            }
        }
        right.parent = parent.self_pos;
        
        page result=node;
        if(val>=mid)
            result=right;
        
        writeBackNode(node);
        writeBackNode(parent);
        writeBackNode(right);
        
        return result;
    }

    void insertData(page node, long long val, node_position source)
    {
        if(pageSize(node.self_pos)==DU*2-1)
        {
            node = splitPage(node,val);
        }

        int index=0;
        for(;index<2*DU-1;index++)
        {
            if(node.data[index]==0||node.data[index]>val)
                break;
        }
        if(node.isLeaf==0)
        {
            if(!(node.child[index].pageNum == 0 && node.child[index].posNum==0))
            {
                insertData(getNode(node.child[index].pageNum, node.child[index].posNum), val, source);
                return;
            }
            else
            {
                node.data[index-1] = val;
                writeBackNode(node);
                insertData(getNode(node.child[index-1].pageNum, node.child[index-1].posNum), val, source);
                return;
            }
        }

        if(pageSize(node.self_pos)>0)
            shiftRight(node,index,1);

        node = getNode(node.self_pos.pageNum, node.self_pos.posNum);;
        node.data[index]=val;
        node.child[index] = source;

        writeBackNode(node);
    }
    
    void addData(long long val , node_position pos)
    {
        insertData(getNode(0,1), val, pos);
    }
    
    search_pos keySearchMax(int pageID, int position, long long key)
    {
        node_position pos;
        pos.pageNum = pageID;
        pos.posNum = position;
        int index = maxIndex(pos);
        int i;
        page now = getNode(pageID, position);
        for(i=0;i<=index;i++)
        {
            if((now.data[i]>key)) break;
        }

        search_pos pp;
        pp.pos.pageNum = 0;
        pp.pos.posNum = 0;
        pp.index = 0;
        if(now.isLeaf == 1)
        {
            if(i > index && now.sibling.pageNum == 0 && now.sibling.posNum == 0)
                return pp;
            else
            {
                if(i>index)
                {
                    pp.pos = now.sibling;
                    pp.index = 0;
                    page sibling = getNode(now.sibling.pageNum, now.sibling.posNum);
                    while(sibling.child[0].pageNum == 0 && sibling.child[0].posNum == 0)
                    {
                        sibling = getNode(sibling.sibling.pageNum, sibling.sibling.posNum);
                    }
                    pp.pos = sibling.self_pos;
                    return pp;
                }
                else
                {
                    pp.pos = now.self_pos;
                    pp.index = i;
                    return pp;
                }
            }
        }
        else
            return keySearchMax(now.child[i].pageNum, now.child[i].posNum, key);
    }
    
    search_pos keySearchMin(int pageID, int position, long long key)
    {
        node_position pos;
        pos.pageNum = pageID;
        pos.posNum = position;
        int index = maxIndex(pos);
        int i;
        page now = getNode(pageID, position);
        for(i=0;i<=index;i++)
        {
            if((now.data[i]>=key)) break;
        }

        search_pos pp;
        pp.pos.pageNum = 0;
        pp.pos.posNum = 0;
        pp.index = 0;
        if(now.isLeaf == 1)
        {
            for(i=index;i>=0;i--)
                if(now.data[i]<key) break;
            i++;
            if(now.child[i].pageNum == 0 && now.child[i].posNum == 0)
            {
                pp.pos = now.sibling;
                pp.index = 0;
                page sibling = getNode(now.sibling.pageNum, now.sibling.posNum);
                while(sibling.child[0].pageNum == 0 && sibling.child[0].posNum == 0)
                {
                    sibling = getNode(sibling.sibling.pageNum, sibling.sibling.posNum);
                }
                pp.pos = sibling.self_pos;
            }
            else
            {
                pp.pos = now.self_pos;
                pp.index = i;
            }
            return pp;
        }
        else
            return keySearchMin(now.child[i].pageNum, now.child[i].posNum, key);
    }
    
    void deleteKey(search_pos pos)
    {
        if(pos.pos.pageNum == 0 && pos.pos.posNum == 0)
            return;
        page node = getNode(pos.pos.pageNum, pos.pos.posNum);
        for(int i=pos.index;i<2*DU-2;i++)
        {
            node.data[i] = node.data[i+1];
            node.child[i] = node.child[i+1];
        }
        node.data[2*DU-2] = 0;
        node.child[2*DU-2].pageNum = 0;
        node.child[2*DU-2].posNum = 0;
        node.child[2*DU-1].pageNum = 0;
        node.child[2*DU-1].posNum = 0;
        writeBackNode(node);
    }
    
    search_pos search_one(search_pos first, search_pos second, int pageNum, int posNum)
    {
        page temp = getNode(first.pos.pageNum, first.pos.posNum);
        search_pos des;
        des.pos.pageNum = 0;
        des.pos.posNum = 0;
        int index = first.index;
        while(1)
        {
            for(;index<2*DU-1;index++)
            {
                if(temp.data[index]==0)
                    break;
                if(temp.child[index].pageNum == pageNum && temp.child[index].posNum == posNum)
                {
                    des.pos = temp.self_pos;
                    des.index = index;
                    break;
                }
            }
            if((temp.sibling.pageNum == 0 && temp.sibling.posNum == 0) || (temp.self_pos.pageNum == second.pos.pageNum && temp.self_pos.posNum == second.pos.posNum) || (des.pos.pageNum != 0 && des.pos.posNum !=0 ))
                break;
            else
            {
                temp = getNode(temp.sibling.pageNum, temp.sibling.posNum);
            }
            index = 0;
        }
        return des;
    }
};

#endif

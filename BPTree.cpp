#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <vector>
#include <iostream>
#include "BPTree.h"
using namespace std;
int depth;

node* createNode() {
	node *newNode;
	newNode = (struct node *)malloc(sizeof(struct node));
	memset(newNode, 0, sizeof(struct node));
	//newNode->isLeaf = isLeaf;
	//newNode->parentNode = parent;
	return newNode;
}

void initNode(node *initialNode) {
	//initialNode->isLeaf = true;
	initialNode->elseNode = NULL;
	initialNode->entryCount = 0;
	depth = 0;
}

void copyNodeEntry(node *dst, node *src, int dstFirst, int srcFirst, int size) {
	size = size * sizeof(struct entry);
	memcpy(&dst->nodeEntry[dstFirst], &src->nodeEntry[srcFirst], size);
}

static struct node* split(node *nodeToSplit, float key, vector<node*> &parentSet, int parentIndex, bool isLeaf) {
	int splitFactor = ((ENTRYSIZE -1) >> 1);
	float splitKey = nodeToSplit->nodeEntry[splitFactor].score;
	
	node *parentNode = NULL;
	node *leftNode = NULL;
	node *rightNode = NULL;
	
	/*leftNode�� ������ ��带 �״�� ���
	rightNode�� ���� �����Ͽ� leftNode��
	split ���� ������ ���ҵ��� ������*/
	leftNode = nodeToSplit;	//
	//rightNode = createNode(nodeToSplit->isLeaf);	
	rightNode = createNode();
	copyNodeEntry(rightNode, nodeToSplit, 0, splitFactor + 1, ENTRYSIZE - splitFactor + 1);
		
	if (isLeaf) {
		rightNode->elseNode = leftNode->elseNode;
		leftNode->elseNode = rightNode;
	}
	else leftNode->elseNode = leftNode->nodeEntry[splitFactor + 1].a.childNode;
	
	/*rightNode�� ����� ���ҵ��� leftNode���� ������*/
	for (int i = splitFactor + 1; i < ENTRYSIZE; i++) {
		memset(&leftNode->nodeEntry[i], 0, sizeof(struct entry));
	}

	leftNode->entryCount = splitFactor + 1;
	rightNode->entryCount = ENTRYSIZE - (splitFactor + 1);
	
	/*ParentNode�� �����ϴ� ����*/
	if (parentSet[parentIndex] == NULL) {   //���ҵǴ� ��尡 ������ ��Ʈ��忴�� ���
		parentNode = createNode(); //ParentNode ����
		root = parentNode;
		depth++;
	}
	
	else { //ParentNode�� ���� ����� ���� ���
		parentNode = parentSet[parentIndex];
		
		/*parentNode�� �� á�� ��� ���ø�*/
		if (parentNode->entryCount == ENTRYSIZE) {
			parentNode = split(parentNode, splitKey, parentSet, parentIndex - 1, false);
		}
	}
	
	/*---------���⿡ parent��忡 ���ԵǴ� �κ� �ۼ�--------*/
	internalNodeInsert(parentNode, splitKey, leftNode, rightNode);

	if (key < splitKey) {
		return leftNode;
	}
	else {
		return rightNode;
	}
}

void internalNodeInsert(node *internalNode, float key, node *leftNode, node *rightNode) {
	entry newEntry;
	blockPointer ptr;
	
	/*���Ե� ��Ʈ���� ����Ű�� �����ʹ� ���� child ���*/
	ptr.childNode = leftNode;
	/*��Ʈ�� ����*/
	//createNewEntry(&newEntry, internalNode->isLeaf, key, ptr);
	createNewEntry(&newEntry, false, key, ptr);
	int index = internalNode->entryCount;
	for (int i = 0; i < internalNode->entryCount; i++) {
		if (key <= internalNode->nodeEntry[i].score) {
			index = i;
			break;
		}
	}
	/*��Ʈ�� ����*/
	for (int i = internalNode->entryCount; i > index; i--) {
		memcpy(&internalNode->nodeEntry[i], &internalNode->nodeEntry[i - 1], sizeof(struct entry)); //��Ʈ���� �ϳ��� right shift
	}
	memset(&internalNode->nodeEntry[index], 0, sizeof(struct entry)); //������ ��Ʈ�� ���� �ʱ�ȭ
	memcpy(&internalNode->nodeEntry[index], &newEntry, sizeof(struct entry)); //��Ʈ�� ����

	/*��Ʈ���� ����� ���� ���� ���Ե� ���*/
	if (index == internalNode->entryCount) {
		internalNode->elseNode = rightNode;
	}
	/*��Ʈ���� ����� �߰� ��� ���Ե� ���*/
	else {
		internalNode->nodeEntry[index + 1].a.childNode = rightNode;
	}
	internalNode->entryCount++;
}

int search(float key) {
	return searchDetail(root, key);
}

int searchDetail(node *currentNode, float key)
{
	int count = 0;
	int currentDepth = 0;
	while (1) {
		//if (currentNode->isLeaf) break;
		if (currentDepth == depth) break;
		if (currentNode->entryCount > 0) {
			for (int i = 0; i < currentNode->entryCount; i++) {
				count++;
				if (key <= currentNode->nodeEntry[i].score) {
					//if (currentNode->isLeaf) break;
					if (currentDepth == depth) break;
					currentNode = currentNode->nodeEntry[i].a.childNode;
					currentDepth++;
					break;
				}
				if (count == currentNode->entryCount) {
					//if (currentNode->isLeaf) break;
					if (currentDepth == depth) break;
					currentNode = currentNode->elseNode;
					currentDepth++;
					break;
				}
			}
			count = 0;
		}
		else
		{
			return -1;
		}
	}
	for (int i = 0; i < currentNode->entryCount; i++) {
		if (key == currentNode->nodeEntry[i].score)
			return currentNode->nodeEntry[i].a.blockNum;
	}
	return -1;
}

void createNewEntry(entry *newEntry, bool isLeaf, float key, blockPointer ptr) {
	newEntry->score = key;
	if (isLeaf) 
		newEntry->a.blockNum = ptr.blockNum;
	else
		newEntry->a.childNode = ptr.childNode;
}

bool insert(float key, blockPointer ptr) {
	entry newEntry;
	node *tmpNode = root;
	int count = 0;
	int index;
	vector<node*> parentSet;
	node *parentNode = NULL;
	int currentDepth = 0;

	/*key���� �̹� Ʈ���� �����ϴ��� üũ*/
	if (search(key) >= 0) {
		return false;
	}

	/*��Ʈ���� ������ ������带 ã�� �ܰ�*/
	while (1) {
		parentSet.push_back(parentNode);
		//if (tmpNode->isLeaf) break;
		if (currentDepth == depth) break;
		if (tmpNode->entryCount > 0) {
			for (int i = 0; i < tmpNode->entryCount; i++) {
				count++;
				if (key <= tmpNode->nodeEntry[i].score) {
					parentNode = tmpNode;
					tmpNode = tmpNode->nodeEntry[i].a.childNode;
					currentDepth++;
					break;
				}
				if (count == tmpNode->entryCount) {
					//if (tmpNode->isLeaf) break;
					if (currentDepth == depth) break;
					parentNode = tmpNode;
					tmpNode = tmpNode->elseNode;
					currentDepth++;
					break;
				}
			}
			count = 0;
		}
	}

	/*�ش� ������ �� á�� ��� ��� ����*/
	if (tmpNode->entryCount == ENTRYSIZE) {
		tmpNode = split(tmpNode, key, parentSet, parentSet.size() - 1, true);
	}

	/*���ο� ��Ʈ�� ����*/
	createNewEntry(&newEntry, true, key, ptr);
	
	/*��忡 ������ ��ġ ����*/
	index = tmpNode->entryCount;
	for (int i = 0; i < tmpNode->entryCount; i++) {
		if (key <= tmpNode->nodeEntry[i].score) {
			index = i;
			break;
		}
	}
	/*��忡 ��Ʈ���� ����*/
	//if (tmpNode->isLeaf) { 
		/*��尡 ������ ���(������ �����ϵ�)*/		
	for (int i = tmpNode->entryCount; i > index; i--) {
		memcpy(&tmpNode->nodeEntry[i], &tmpNode->nodeEntry[i - 1], sizeof(struct entry)); //��Ʈ���� �ϳ��� right shift
	}
	memset(&tmpNode->nodeEntry[index], 0, sizeof(struct entry)); //������ ��Ʈ�� ���� �ʱ�ȭ
	memcpy(&tmpNode->nodeEntry[index], &newEntry, sizeof(struct entry)); //��Ʈ�� ����
	tmpNode->entryCount++;
	//}

	return true;
}

node* sequencialSearch(int k) {
	node *tmp;
	tmp = root;
	int currentDepth = 0;
	tmp = seqSearchDetail(root, k, currentDepth);
	for (int i = 1; i < k; i++) {
		if (tmp->elseNode != NULL) {
			tmp = tmp->elseNode;
		}
		else
			return NULL;
	}
	return tmp;
}
node* seqSearchDetail(node *currentNode, int k, int currentDepth) {
	if (currentDepth != depth) {
		if (currentNode->entryCount != 0) {
			return seqSearchDetail(currentNode->nodeEntry[0].a.childNode, k, currentDepth + 1);
		}
		else
			return NULL;
	}
	else
		return currentNode;
}




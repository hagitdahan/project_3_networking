#include "List.h"
#include <stdlib.h>
#include <stdio.h>

// Node & List Data Structures
typedef struct _node {
    float _time;
    size_t _weight;
    struct _node * _next;
} Node;


struct _List {
    Node* _head;
    size_t _size;
};


//------------------------------------------------
// Node implementation
//------------------------------------------------

Node* Node_alloc(float data,size_t weight,Node* next) {
	Node* p= (Node*)malloc(sizeof(Node));
	p->_time= data;
    p->_weight = weight;
	p->_next= next;
	return p;
}

void Node_free(Node* node) {
	free(node);
}
//------------------------------------------------



//------------------------------------------------
// List implementation
//------------------------------------------------

List* List_alloc() {
	List* p= (List*)malloc(sizeof(List));
	p->_head= NULL;
	p->_size= 0;
	return p;
}

void List_free(List* list) {
	if (list==NULL) return;
	Node* p1= list->_head;
	Node* p2;
	while(p1) {
		p2= p1;
		p1= p1->_next;
		Node_free(p2);
	}
	free(list);
}

size_t List_size(const List* list) {
	return list->_size;
}

void List_insertLast(List* list,float data,size_t weight){
    Node* p = list->_head;
    if(!p){
        list ->_head = Node_alloc(data,weight,NULL);
        list->_size++;
        return;
    }
    while(p->_next){
        p=p->_next;
    }
    p->_next = Node_alloc(data,weight,NULL);
    list->_size++;
}

void List_print(const List* list) {
	const Node* p= list->_head;
    int i=0;
	while(p) {
        float speed = ((float)(p->_weight)/(p->_time))*0.008; // how to calculate
		printf("- Run #%d data: Time:%fms; Speed=%fMB/s\n",i,p->_time,speed);
		p= p->_next;
        i++;
	}
}

float List_avarage_time(const List* list) {
    float result =0;
    const Node* p= list->_head;
    while(p) {
        result+= p->_time;
        p= p->_next;
    }
    return result/(float)list->_size;
}

float List_avarage_bandwidth(const List* list) {
    float result =0;
    const Node* p= list->_head;
    while(p) {
        float speed = ((float)(p->_weight)/(p->_time))*0.008; // how to calculate
        result+= speed;
        p= p->_next;
    }
    return result/(float)list->_size;
}

//------------------------------------------------

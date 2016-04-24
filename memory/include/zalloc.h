/*define the general m_allocator*/
struct mmblock{
	struct mmblock * prev;
	struct mmblock * next;
	int size;
};
typedef struct{
	int mx_sz;  /*total size of the allocator*/
	struct mmblock * head; /*head node of the memory list */
}m_allocator;

m_allocator * m_allocator_construct(int size);
void m_allocator_destruct();






//
//  main.c
//  analy_inode
//
//  Created by 丁贵强 on 16/4/15.
//  Copyright © 2016年 丁贵强. All rights reserved.
//

#include <stdio.h>
#include "common.h"


/* All arch specific implementations share the same struct */
struct rw_semaphore {
    long            count;
    raw_spinlock_t    wait_lock;
    struct list_head    wait_list;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
    struct lockdep_map    dep_map;
#endif
};


typedef struct _rwlock {
    int32_t        readers;    /* rwstate word */
    uint16_t    type;
    uint16_t    magic;
    mutex_t        mutex;        /* used with process-shared rwlocks */
    cond_t        readercv;    /* used only to indicate ownership */
    cond_t        writercv;    /* used only to indicate ownership */
} rwlock_t;


struct address_space {
    struct inode           *host;            /* owner: inode, block_device拥有它的节点 */
    struct radix_tree_root    page_tree;       /* radix tree of all pages包含全部页面的radix树 */
    rwlock_t        tree_lock;       /* and rwlock protecting it保护page_tree的自旋锁  */
    unsigned int           i_mmap_writable;/* count VM_SHARED mappings共享映射数 VM_SHARED记数*/
    struct prio_tree_root      i_mmap;         /* tree of private and shared mappings 优先搜索树的树根*/
    struct list_head       i_mmap_nonlinear;/*list VM_NONLINEAR mappings 非线性映射的链表头*/
    spinlock_t              i_mmap_lock; /* protect tree, count, list 保护i_mmap的自旋锁*/
    unsigned int           truncate_count;      /* Cover race condition with truncate 将文件截断的记数*/
    unsigned long         nrpages;  /* number of total pages 页总数*/
    pgoff_t                  writeback_index;/* writeback starts here 回写的起始偏移*/
    struct address_space_operations *a_ops;     /* methods  操作函数表*/
    unsigned long         flags;             /* error bits/gfp mask ，gfp_mask掩码与错误标识 */
    struct backing_dev_info *backing_dev_info; /* device readahead, etc预读信息 */
    spinlock_t              private_lock;   /* for use by the address_space  私有address_space锁*/
    struct list_head       private_list;     /* ditto 私有address_space链表*/
    struct address_space     *assoc_mapping;    /* ditto 相关的缓冲*/
} __attribute__((aligned(sizeof(long))));


typedef unsigned int __u32;


struct inode {
    struct hlist_node    i_hash;
    struct list_head    i_list;
    struct list_head    i_sb_list;
    struct list_head    i_dentry;
    unsigned long        i_ino;
    atomic_t        i_count;
    unsigned int        i_nlink;
    uid_t            i_uid;
    gid_t            i_gid;
    dev_t            i_rdev;   //该成员表示设备文件的inode结构，它包含了真正的设备编号。
    u64            i_version;
    loff_t            i_size;
#ifdef __NEED_I_SIZE_ORDERED
    seqcount_t        i_size_seqcount;
#endif
    struct timespec        i_atime;
    struct timespec        i_mtime;
    struct timespec        i_ctime;
    unsigned int        i_blkbits;
    blkcnt_t        i_blocks;
    unsigned short          i_bytes;
    umode_t            i_mode;
    spinlock_t        i_lock;    /* i_blocks, i_bytes, maybe i_size */
    struct mutex        i_mutex;
    struct rw_semaphore    i_alloc_sem;
    const struct inode_operations    *i_op;
    const struct file_operations    *i_fop;    /* former ->i_op->default_file_ops */
    struct super_block    *i_sb;
    struct file_lock    *i_flock;
    struct address_space    *i_mapping;
    struct address_space    i_data;
#ifdef CONFIG_QUOTA
    struct dquot        *i_dquot[MAXQUOTAS];
#endif
    struct list_head    i_devices;
    union {
        struct pipe_inode_info    *i_pipe;
        struct block_device    *i_bdev;
        struct cdev        *i_cdev; //该成员表示字符设备的内核的 内部结构。当inode指向一个字符设备文件时，该成员包含了指向struct cdev结构的指针，其中cdev结构是字符设备结构体。
     };
    int            i_cindex;
    
    __u32            i_generation;
    
#ifdef CONFIG_DNOTIFY
    unsigned long        i_dnotify_mask; /* Directory notify events */
    struct dnotify_struct    *i_dnotify; /* for directory notifications */
#endif
    
#ifdef CONFIG_INOTIFY
    struct list_head    inotify_watches; /* watches on this inode */
    struct mutex        inotify_mutex;    /* protects the watches list */
#endif
    
    unsigned long        i_state;
    unsigned long        dirtied_when;    /* jiffies of first dirtying */
    unsigned int        i_flags;
    atomic_t        i_writecount;
#ifdef CONFIG_SECURITY
    void            *i_security;
#endif
    void            *i_private; /* fs or device private pointer */
};


struct dentry {
    atomic_t d_count; //目录项对象使用计数器
    unsigned int d_flags; //目录项标志
    struct inode * d_inode; //与文件名关联的索引节点
    struct dentry * d_parent; //父目录的目录项对象
    struct list_head d_hash;// 散列表表项的指针
    struct list_head d_lru; //未使用链表的指针
    struct list_head d_child; //父目录中目录项对象的链表的指针
    struct list_head d_subdirs;//对目录而言，表示子目录目录项对象的链表
    struct list_head d_alias; //相关索引节点（别名）的链表
    int d_mounted; //对于安装点而言，表示被安装文件系统根项
    struct qstr d_name;// 文件名
    unsigned long d_time; /* used by d_revalidate */
    struct dentry_operations *d_op; //目录项方法
    struct super_block * d_sb; //文件的超级块对象
    vunsigned long d_vfs_flags;
    void * d_fsdata;//与文件系统相关的数据
    unsigned char d_iname [DNAME_INLINE_LEN]; //存放短文件名
};


static int __ext2_write_inode(struct inode *inode, int do_sync);
static inline int ext2_inode_is_fast_symlink(struct inode *inode)
{
}


#include <linux/time.h>
#include <linux/highuid.h>
#include <linux/pagemap.h>
#include <linux/dax.h>
#include <linux/quotaops.h>
#include <linux/writeback.h>
#include <linux/buffer_head.h>
#include <linux/mpage.h>
#include <linux/fiemap.h>
#include <linux/namei.h>
#include <linux/uio.h>
#include "ext2.h"
#include "acl.h"
#include "xattr.h"

static int __ext2_write_inode(struct inode *inode, int do_sync);

/*
 * Test whether an inode is a fast symlink.
 */
static inline int ext2_inode_is_fast_symlink(struct inode *inode)
{
   
}

static void ext2_truncate_blocks(struct inode *inode, loff_t offset);

static void ext2_write_failed(struct address_space *mapping, loff_t to)
{
}

/*
 * Called at the last iput() if i_nlink is zero.
 */
void ext2_evict_inode(struct inode * inode)
{
   
}


typedef struct {
    __le32	*p;
    __le32	key;
    struct buffer_head *bh;
} Indirect;

static inline void add_chain(Indirect *p, struct buffer_head *bh, __le32 *v)
{
   
}

static inline int verify_chain(Indirect *from, Indirect *to)
{
    
}

/**
 *	ext2_block_to_path - parse the block number into array of offsets
 *	@inode: inode in question (we are only interested in its superblock)
 *	@i_block: block number to be parsed
 *	@offsets: array to store the offsets in
 *      @boundary: set this non-zero if the referred-to block is likely to be
 *             followed (on disk) by an indirect block.
 *	To store the locations of file's data ext2 uses a data structure common
 *	for UNIX filesystems - tree of pointers anchored in the inode, with
 *	data blocks at leaves and indirect blocks in intermediate nodes.
 *	This function translates the block number into path in that tree -
 *	return value is the path length and @offsets[n] is the offset of
 *	pointer to (n+1)th node in the nth one. If @block is out of range
 *	(negative or too large) warning is printed and zero returned.
 *
 *	Note: function doesn't find node addresses, so no IO is needed. All
 *	we need to know is the capacity of indirect blocks (taken from the
 *	inode->i_sb).
 */

/*
 * Portability note: the last comparison (check that we fit into triple
 * indirect block) is spelled differently, because otherwise on an
 * architecture with 32-bit longs and 8Kb pages we might get into trouble
 * if our filesystem had 8Kb blocks. We might use long long, but that would
 * kill us on x86. Oh, well, at least the sign propagation does not matter -
 * i_block would have to be negative in the very beginning, so we would not
 * get there at all.
 */

static int ext2_block_to_path(struct inode *inode,
                              long i_block, int offsets[4], int *boundary)
{
    
}
/**
 *	ext2_get_branch - read the chain of indirect blocks leading to data
 *	@inode: inode in question
 *	@depth: depth of the chain (1 - direct pointer, etc.)
 *	@offsets: offsets of pointers in inode/indirect blocks
 *	@chain: place to store the result
 *	@err: here we store the error value
 *
 *	Function fills the array of triples <key, p, bh> and returns %NULL
 *	if everything went OK or the pointer to the last filled triple
 *	(incomplete one) otherwise. Upon the return chain[i].key contains
 *	the number of (i+1)-th block in the chain (as it is stored in memory,
 *	i.e. little-endian 32-bit), chain[i].p contains the address of that
 *	number (it points into struct inode for i==0 and into the bh->b_data
 *	for i>0) and chain[i].bh points to the buffer_head of i-th indirect
 *	block for i>0 and NULL for i==0. In other words, it holds the block
 *	numbers of the chain, addresses they were taken from (and where we can
 *	verify that chain did not change) and buffer_heads hosting these
 *	numbers.
 *
 *	Function stops when it stumbles upon zero pointer (absent block)
 *		(pointer to last triple returned, *@err == 0)
 *	or when it gets an IO error reading an indirect block
 *		(ditto, *@err == -EIO)
 *	or when it notices that chain had been changed while it was reading
 *		(ditto, *@err == -EAGAIN)
 *	or when it reads all @depth-1 indirect blocks successfully and finds
 *	the whole chain, all way to the data (returns %NULL, *err == 0).
 */
static Indirect *ext2_get_branch(struct inode *inode,
                                 int depth,
                                 int *offsets,
                                 Indirect chain[4],
                                 int *err)
{
}

/**
 *	ext2_find_near - find a place for allocation with sufficient locality
 *	@inode: owner
 *	@ind: descriptor of indirect block.
 *
 *	This function returns the preferred place for block allocation.
 *	It is used when heuristic for sequential allocation fails.
 *	Rules are:
 *	  + if there is a block to the left of our position - allocate near it.
 *	  + if pointer will live in indirect block - allocate near that block.
 *	  + if pointer will live in inode - allocate in the same cylinder group.
 *
 * In the latter case we colour the starting block by the callers PID to
 * prevent it from clashing with concurrent allocations for a different inode
 * in the same block group.   The PID is used here so that functionally related
 * files will be close-by on-disk.
 *
 *	Caller must make sure that @ind is valid and will stay that way.
 */

static ext2_fsblk_t ext2_find_near(struct inode *inode, Indirect *ind)
{
    
}

/**
 *	ext2_find_goal - find a preferred place for allocation.
 *	@inode: owner
 *	@block:  block we want
 *	@partial: pointer to the last triple within a chain
 *
 *	Returns preferred place for a block (the goal).
 */

static inline ext2_fsblk_t ext2_find_goal(struct inode *inode, long block,
                                          Indirect *partial)
{
   
}

/**
 *	ext2_blks_to_allocate: Look up the block map and count the number
 *	of direct blocks need to be allocated for the given branch.
 *
 * 	@branch: chain of indirect blocks
 *	@k: number of blocks need for indirect blocks
 *	@blks: number of data blocks to be mapped.
 *	@blocks_to_boundary:  the offset in the indirect block
 *
 *	return the total number of blocks to be allocate, including the
 *	direct and indirect blocks.
 */
static int
ext2_blks_to_allocate(Indirect * branch, int k, unsigned long blks,
                      int blocks_to_boundary)
{
}

/**
 *	ext2_alloc_blocks: multiple allocate blocks needed for a branch
 *	@indirect_blks: the number of blocks need to allocate for indirect
 *			blocks
 *
 *	@new_blocks: on return it will store the new block numbers for
 *	the indirect blocks(if needed) and the first direct block,
 *	@blks:	on return it will store the total number of allocated
 *		direct blocks
 */
static int ext2_alloc_blocks(struct inode *inode,
                             ext2_fsblk_t goal, int indirect_blks, int blks,
                             ext2_fsblk_t new_blocks[4], int *err)
{
   
}

/**
 *	ext2_alloc_branch - allocate and set up a chain of blocks.
 *	@inode: owner
 *	@num: depth of the chain (number of blocks to allocate)
 *	@offsets: offsets (in the blocks) to store the pointers to next.
 *	@branch: place to store the chain in.
 *
 *	This function allocates @num blocks, zeroes out all but the last one,
 *	links them into chain and (if we are synchronous) writes them to disk.
 *	In other words, it prepares a branch that can be spliced onto the
 *	inode. It stores the information about that chain in the branch[], in
 *	the same format as ext2_get_branch() would do. We are calling it after
 *	we had read the existing part of chain and partial points to the last
 *	triple of that (one with zero ->key). Upon the exit we have the same
 *	picture as after the successful ext2_get_block(), except that in one
 *	place chain is disconnected - *branch->p is still zero (we did not
 *	set the last link), but branch->key contains the number that should
 *	be placed into *branch->p to fill that gap.
 *
 *	If allocation fails we free all blocks we've allocated (and forget
 *	their buffer_heads) and return the error value the from failed
 *	ext2_alloc_block() (normally -ENOSPC). Otherwise we set the chain
 *	as described above and return 0.
 */

static int ext2_alloc_branch(struct inode *inode,
                             int indirect_blks, int *blks, ext2_fsblk_t goal,
                             int *offsets, Indirect *branch)
{
}

/**
 * ext2_splice_branch - splice the allocated branch onto inode.
 * @inode: owner
 * @block: (logical) number of block we are adding
 * @where: location of missing link
 * @num:   number of indirect blocks we are adding
 * @blks:  number of direct blocks we are adding
 *
 * This function fills the missing link and does all housekeeping needed in
 * inode (->i_blocks, etc.). In case of success we end up with the full
 * chain to new block and return 0.
 */
static void ext2_splice_branch(struct inode *inode,
                               long block, Indirect *where, int num, int blks)
{
   
}

/*
 * Allocation strategy is simple: if we have to allocate something, we will
 * have to go the whole way to leaf. So let's do it before attaching anything
 * to tree, set linkage between the newborn blocks, write them if sync is
 * required, recheck the path, free and repeat if check fails, otherwise
 * set the last missing link (that will protect us from any truncate-generated
 * removals - all blocks on the path are immune now) and possibly force the
 * write on the parent block.
 * That has a nice additional property: no special recovery from the failed
 * allocations is needed - we simply release blocks and do not touch anything
 * reachable from inode.
 *
 * `handle' can be NULL if create == 0.
 *
 * return > 0, # of blocks mapped or allocated.
 * return = 0, if plain lookup failed.
 * return < 0, error case.
 */
static int ext2_get_blocks(struct inode *inode,
                           sector_t iblock, unsigned long maxblocks,
                           struct buffer_head *bh_result,
                           int create)
{
}

int ext2_get_block(struct inode *inode, sector_t iblock, struct buffer_head *bh_result, int create)
{
  
    
}

int ext2_fiemap(struct inode *inode, struct fiemap_extent_info *fieinfo,
                u64 start, u64 len)
{

}

static int ext2_writepage(struct page *page, struct writeback_control *wbc)
{
    
}

static int ext2_readpage(struct file *file, struct page *page)
{
    
}

static int
ext2_readpages(struct file *file, struct address_space *mapping,
               struct list_head *pages, unsigned nr_pages)
{
    
}

static int
ext2_write_begin(struct file *file, struct address_space *mapping,
                 loff_t pos, unsigned len, unsigned flags,
                 struct page **pagep, void **fsdata)
{
}

static int ext2_write_end(struct file *file, struct address_space *mapping,
                          loff_t pos, unsigned len, unsigned copied,
                          struct page *page, void *fsdata)
{
   
}

static int
ext2_nobh_write_begin(struct file *file, struct address_space *mapping,
                      loff_t pos, unsigned len, unsigned flags,
                      struct page **pagep, void **fsdata)
{
}

static int ext2_nobh_writepage(struct page *page,
                               struct writeback_control *wbc)
{
    
}

static sector_t ext2_bmap(struct address_space *mapping, sector_t block)
{
   
}

static ssize_t
ext2_direct_IO(struct kiocb *iocb, struct iov_iter *iter, loff_t offset)
{
}

static int
ext2_writepages(struct address_space *mapping, struct writeback_control *wbc)
{

}

const struct address_space_operations ext2_aops = {
    .readpage		= ext2_readpage,
    .readpages		= ext2_readpages,
    .writepage		= ext2_writepage,
    .write_begin		= ext2_write_begin,
    .write_end		= ext2_write_end,
    .bmap			= ext2_bmap,
    .direct_IO		= ext2_direct_IO,
    .writepages		= ext2_writepages,
    .migratepage		= buffer_migrate_page,
    .is_partially_uptodate	= block_is_partially_uptodate,
    .error_remove_page	= generic_error_remove_page,
};

const struct address_space_operations ext2_nobh_aops = {
    .readpage		= ext2_readpage,
    .readpages		= ext2_readpages,
    .writepage		= ext2_nobh_writepage,
    .write_begin		= ext2_nobh_write_begin,
    .write_end		= nobh_write_end,
    .bmap			= ext2_bmap,
    .direct_IO		= ext2_direct_IO,
    .writepages		= ext2_writepages,
    .migratepage		= buffer_migrate_page,
    .error_remove_page	= generic_error_remove_page,
};

/*
 * Probably it should be a library function... search for first non-zero word
 * or memcmp with zero_page, whatever is better for particular architecture.
 * Linus?
 */
static inline int all_zeroes(__le32 *p, __le32 *q)
{
}

/**
 *	ext2_find_shared - find the indirect blocks for partial truncation.
 *	@inode:	  inode in question
 *	@depth:	  depth of the affected branch
 *	@offsets: offsets of pointers in that branch (see ext2_block_to_path)
 *	@chain:	  place to store the pointers to partial indirect blocks
 *	@top:	  place to the (detached) top of branch
 *
 *	This is a helper function used by ext2_truncate().
 *
 *	When we do truncate() we may have to clean the ends of several indirect
 *	blocks but leave the blocks themselves alive. Block is partially
 *	truncated if some data below the new i_size is referred from it (and
 *	it is on the path to the first completely truncated data block, indeed).
 *	We have to free the top of that path along with everything to the right
 *	of the path. Since no allocation past the truncation point is possible
 *	until ext2_truncate() finishes, we may safely do the latter, but top
 *	of branch may require special attention - pageout below the truncation
 *	point might try to populate it.
 *
 *	We atomically detach the top of branch from the tree, store the block
 *	number of its root in *@top, pointers to buffer_heads of partially
 *	truncated blocks - in @chain[].bh and pointers to their last elements
 *	that should not be removed - in @chain[].p. Return value is the pointer
 *	to last filled element of @chain.
 *
 *	The work left to caller to do the actual freeing of subtrees:
 *		a) free the subtree starting from *@top
 *		b) free the subtrees whose roots are stored in
 *			(@chain[i].p+1 .. end of @chain[i].bh->b_data)
 *		c) free the subtrees growing from the inode past the @chain[0].p
 *			(no partially truncated stuff there).
 */

static Indirect *ext2_find_shared(struct inode *inode,
                                  int depth,
                                  int offsets[4],
                                  Indirect chain[4],
                                  __le32 *top)
{
}

/**
 *	ext2_free_data - free a list of data blocks
 *	@inode:	inode we are dealing with
 *	@p:	array of block numbers
 *	@q:	points immediately past the end of array
 *
 *	We are freeing all blocks referred from that array (numbers are
 *	stored as little-endian 32-bit) and updating @inode->i_blocks
 *	appropriately.
 */
static inline void ext2_free_data(struct inode *inode, __le32 *p, __le32 *q)
{
   }

/**
 *	ext2_free_branches - free an array of branches
 *	@inode:	inode we are dealing with
 *	@p:	array of block numbers
 *	@q:	pointer immediately past the end of array
 *	@depth:	depth of the branches to free
 *
 *	We are freeing all blocks referred from these branches (numbers are
 *	stored as little-endian 32-bit) and updating @inode->i_blocks
 *	appropriately.
 */
static void ext2_free_branches(struct inode *inode, __le32 *p, __le32 *q, int depth)
{
}

/* dax_sem must be held when calling this function */
static void __ext2_truncate_blocks(struct inode *inode, loff_t offset)
{
   
}

static void ext2_truncate_blocks(struct inode *inode, loff_t offset)
{
    /*
     * XXX: it seems like a bug here that we don't allow
     * IS_APPEND inode to have blocks-past-i_size trimmed off.
     * review and fix this.
     *
     * Also would be nice to be able to handle IO errors and such,
     * but that's probably too much to ask.
     */
  }

static int ext2_setsize(struct inode *inode, loff_t newsize)
{
    
}

static struct ext2_inode *ext2_get_inode(struct super_block *sb, ino_t ino,
                                         struct buffer_head **p)
{
}

void ext2_set_inode_flags(struct inode *inode)
{
}

/* Propagate flags from i_flags to EXT2_I(inode)->i_flags */
void ext2_get_inode_flags(struct ext2_inode_info *ei)
{
   
}

struct inode *ext2_iget (struct super_block *sb, unsigned long ino)
{

}

static int __ext2_write_inode(struct inode *inode, int do_sync)
{
}

int ext2_write_inode(struct inode *inode, struct writeback_control *wbc)
{
   
}

int ext2_setattr(struct dentry *dentry, struct iattr *iattr)
{
}




#include "lfs.h"
#include "sd.h"
#include <string.h>

static const unsigned int BLOCK_SIZE = 512;

///// Block device operations //////
static int lfs_bd_read(const struct lfs_config *c, lfs_block_t block,
					   lfs_off_t off, void *buffer, lfs_size_t size)
{
//	BlockDevice *bd = (BlockDevice *)c->context;
//	return bd->read(buffer, (bd_addr_t)block*c->block_size + off, size);
	return SDReadBlocks(buffer, block, size / BLOCK_SIZE, 1000) ? 0 : size;
}

static int lfs_bd_prog(const struct lfs_config *c, lfs_block_t block,
					   lfs_off_t off, const void *buffer, lfs_size_t size)
{
//	BlockDevice *bd = (BlockDevice *)c->context;
//	return bd->program(buffer, (bd_addr_t)block*c->block_size + off, size);
	return 0;
}

static int lfs_bd_erase(const struct lfs_config *c, lfs_block_t block)
{
//	BlockDevice *bd = (BlockDevice *)c->context;
//	return bd->erase((bd_addr_t)block*c->block_size, c->block_size);
	return 0;
}

static int lfs_bd_sync(const struct lfs_config *c)
{
//	BlockDevice *bd = (BlockDevice *)c->context;
//	return bd->sync();
	return 0;
}

void test_lfs(void)
{
	lfs_t fs;
	memset(&fs, 0, sizeof(fs));

	struct lfs_config cfg;
	memset(&cfg, 0, sizeof(cfg));

	cfg.context = 0;
	cfg.read = lfs_bd_read;
	cfg.prog = lfs_bd_prog;
	cfg.erase = lfs_bd_erase;
	cfg.sync = lfs_bd_sync;
	cfg.read_size = BLOCK_SIZE;
	cfg.prog_size = BLOCK_SIZE;
	cfg.block_size = BLOCK_SIZE;
//	cfg.block_count = hbd.size() / cfg.block_size;
//	cfg.lookahead = 32 * ((cfg.block_count + 31) / 32);
	cfg.block_count = 32*1024*1024;
	cfg.lookahead = 32 * 1024;

	int res = lfs_format(&fs, &cfg);

	res = lfs_mount(&fs, &cfg);

	lfs_file_t ff;
	res = lfs_file_open(&fs, &ff, "file1", LFS_O_WRONLY | LFS_O_CREAT);

	res = lfs_file_write(&fs, &ff, "Hello World!", 12);
	res = lfs_file_close(&fs, &ff);
}

#include "lfs.h"
#include "sd.h"
#include <string.h>

static const unsigned int SD_BLOCK_SIZE = 512;

///// Block device operations //////
static int sdRead(const struct lfs_config *c, lfs_block_t block,
					   lfs_off_t off, void *buffer, lfs_size_t size)
{
	//	ASSERT_DBG(0 == off);
	//	ASSERT_DBG(0 == size % SD_BLOCK_SIZE);
	return SDReadBlocks(buffer, block, size / SD_BLOCK_SIZE);
}

static int sdWrite(const struct lfs_config *c, lfs_block_t block,
					   lfs_off_t off, const void *buffer, lfs_size_t size)
{
	//	ASSERT_DBG(0 == off);
	//	ASSERT_DBG(0 == size % SD_BLOCK_SIZE);
	return SDWriteBlocks(buffer, block, size / SD_BLOCK_SIZE);
}

static int sdErase(const struct lfs_config *c, lfs_block_t block)
{
	return SDErase(block, block);
}

static int sdSync(const struct lfs_config *c)
{
	return 0;
}

void test_lfs(void)
{
	lfs_t fs;
	memset(&fs, 0, sizeof(fs));

	struct lfs_config cfg;
	memset(&cfg, 0, sizeof(cfg));

	cfg.context = 0; // transparent context
	cfg.read = sdRead;
	cfg.prog = sdWrite;
	cfg.erase = sdErase;
	cfg.sync = sdSync;
	cfg.read_size = SD_BLOCK_SIZE;
	cfg.prog_size = SD_BLOCK_SIZE;
	cfg.block_size = SD_BLOCK_SIZE;
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

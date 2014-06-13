#include <linux/module.h>
#include <linux/device-mapper.h>

#define DM_MSG_PREFIX "rot13"

struct dm_rot13_c {
	struct dm_dev *dev;
	sector_t start;
};

static int rot13_ctr(struct dm_target *target, unsigned int argc, char **argv)
{
	struct dm_rot13_c *c = NULL;

	if (argc != 1) {
		target->error = "Invalid arguments. expected target device";
		return -EINVAL;
	}

	c = kmalloc(sizeof(*c), GFP_KERNEL);
	if (c == NULL)
		return -ENOMEM;

	c->start = 0;
	if (dm_get_device(target, argv[0],
	    dm_table_get_mode(target->table), &c->dev)) {
		target->error = "dm-" DM_MSG_PREFIX ": device lookup failed";
		goto error;
	}

	target->private = c;
	return 0;

error:
	kfree(c);
	return -EINVAL;
}

static inline void do_rot13(char *data, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (data[i] >= 'a' && data[i] <= 'z')
			data[i] = abs('z' - data[i]) + 'a';
		if (data[i] >= 'A' && data[i] <= 'Z')
			data[i] = abs('Z' - data[i]) + 'A';
	}
}

static void do_rot13_bio(struct bio *bio)
{
	unsigned long flags;
	struct bio_vec *bv;
	int i;

	bio_for_each_segment(bv, bio, i) {
		char *data = bvec_kmap_irq(bv, &flags);
		do_rot13(data, bv->bv_len);
		bvec_kunmap_irq(data, &flags);
	}
}

static int rot13_end_io(struct dm_target *t, struct bio *bio, int error)
{
	do_rot13_bio(bio);
	return 0;
}

static int rot13_map(struct dm_target *t, struct bio *bio)
{
	struct dm_rot13_c *c = (struct dm_rot13_c *)t->private;
	int ret = -EIO;

	bio->bi_bdev = c->dev->bdev;
	switch (bio_rw(bio)) {
		case WRITE:
			do_rot13_bio(bio);
			ret = DM_MAPIO_REMAPPED;
			break;
		case READA:
			ret = -EIO;
			break;
		case READ:
			ret = DM_MAPIO_REMAPPED;
			break;
	}
	return ret;
}

static void rot13_dtr(struct dm_target *ti)
{
	struct dm_rot13_c *c = (struct dm_rot13_c *)ti->private;

	dm_put_device(ti, c->dev);
	kfree(c);
}

static struct target_type rot13_target = {
	.name = DM_MSG_PREFIX,
	.version = {1, 0, 0},
	.module = THIS_MODULE,
	.ctr = rot13_ctr,
	.map = rot13_map,
	.dtr = rot13_dtr,
	.end_io = rot13_end_io
};

static int __init dm_rot13_init(void)
{
	int r = dm_register_target(&rot13_target);

	if (r < 0)
		DMERR("register failed: %d", r);
	return r;
}

static void __exit dm_rot13_exit(void)
{
	dm_unregister_target(&rot13_target);
}

module_init(dm_rot13_init);
module_exit(dm_rot13_exit);

MODULE_AUTHOR("Andrea Righi <andrea@betterlinux.com>");
MODULE_DESCRIPTION("Device mapper rot13 target");
MODULE_LICENSE("GPL");

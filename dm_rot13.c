#include <linux/module.h>
#include <linux/device-mapper.h>
#define DM_MSG_PREFIX "rot13"

struct dm_rot13_c {
	struct dm_dev *dev;
	sector_t start;
};

static int 
rot13_ctr(struct dm_target *target, unsigned int argc, char **argv) {
	struct dm_rot13_c *c = NULL;

	if (argc != 1) {
		target->error = "Invalid arguments. expected target device";
		return -EINVAL;
	}

	c = kmalloc(sizeof(*c), GFP_KERNEL);
	if(c == NULL) {
		return -ENOMEM;
	}

	c->start = 0;
	if(dm_get_device(target, argv[0], dm_table_get_mode(target->table), &c->dev)) {
		target->error = "dm-rot13: device lookup failed";
		goto error;
	}

	target->private = c;
	return 0;

error:
	kfree(c);
	return -EINVAL;
}

static int 
rot13_map(struct dm_target *t, struct bio *bio, union map_info *map_context) {
	unsigned long flags;
	struct bio_vec *bv;
	int i, j, ret;

	struct dm_rot13_c *c = (struct dm_rot13_c *) t->private;
	bio->bi_bdev = c->dev->bdev;
	switch(bio_rw(bio)) {
		case WRITE:
			bio_for_each_segment(bv, bio, i) {
				char *data = bvec_kmap_irq(bv, &flags);
				for(j = 0; j < bv->bv_len; j++) {
					if(data[j] >= 'a' && data[j] <= 'z') {
						data[j] = abs('z' - data[j]) + 'a';
					}
					if(data[j] >= 'A' && data[j] <= 'Z') {
						data[j] = abs('Z' - data[j]) + 'A';
					}
				}
				bvec_kunmap_irq(data, &flags);
			}
			ret = DM_MAPIO_REMAPPED;
			break;
		case READA:
			printk("readahead attempted\n");
			return -EIO;
		case READ:
			dump_stack();
			printk("read %d bvecs\n", bio->bi_vcnt);
			ret = DM_MAPIO_REMAPPED;
	}
	return ret;
}

static void 
rot13_dtr(struct dm_target *ti) {
	struct dm_rot13_c *c = (struct dm_rot13_c *) ti->private;
	dm_put_device(ti, c->dev);
	kfree(c);
}

static struct target_type rot13_target = {
	.name = "rot13",
	.version = {1, 0, 0},
	.module = THIS_MODULE,
	.ctr = rot13_ctr,
	.map = rot13_map,
	.dtr = rot13_dtr
};

static int __init dm_rot13_init(void) {
	int r = dm_register_target(&rot13_target);

	if(r < 0) DMERR("register failed: %d", r);
	return r;
}

static void __exit dm_rot13_exit(void) {
	dm_unregister_target(&rot13_target);
}

module_init(dm_rot13_init);
module_exit(dm_rot13_exit);

MODULE_AUTHOR("Abhijit Hoskeri <abhjiithoskeri@gmail.com>");
MODULE_DESCRIPTION(" device mapper rot13 target");
MODULE_LICENSE("GPL");

#include <linux/module.h>
#include <linux/device-mapper.h>
#define DM_MSG_PREFIX "rot13"

static int rot13_ctr(struct dm_target *target, unsigned int argc, char **argv) {
	return 0;
}

static int rot13_map(struct dm_target *t, struct bio *bio, union map_info *map_context) {
	return 0; /* we will resubmit the io later */
}

static struct target_type rot13_target = {
	.name = "rot13",
	.version = {1, 0, 0},
	.module = THIS_MODULE,
	.ctr = rot13_ctr,
	.map = rot13_map
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

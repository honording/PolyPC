static int __init ddr_malloc_init(void)
{
    
}
static int __exit ddr_malloc_exit(void)
{
    return 0
}
module_init(ddr_malloc_init);
module_init(ddr_malloc_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HaPara: Interface for users to allocate DDR memory.")
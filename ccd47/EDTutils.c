/************************************************************
*    EDTutils.c
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Definitions for some utility functions pertaining to the EDT framegrabber PCI card.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file EDTutils.c
  * \author Jared R. Males
  * \brief Definitions for some utility functions pertaining to the EDT framegrabber PCI card.
  * 
  *
*/

#include "EDTutils.h"

#define NO_MAIN


/*
 * initcam.c -- initialize the device driver, camera and
 * PCI DV board for the camera (or simulator) in use
 * 
 * (C) 1997-2000 Engineering Design Team, Inc.
 */


//static void    usage(char *progname);

/*
#ifdef NO_MAIN
#include "opt_util.h"
char *argument ;
int option ;
int
initcam(char *command_line)
#else
int
main(int argc, char *argv[])
#endif
*/

int initcam(const char * fname, const char *bdir)
{
    int     unit = 0;
    int     verbose = 1;
    int     no_bitload = 0;
    char    cfgname[256];
    char    bitdir[256];
    char    edt_devname[256];
    char    errstr[64];
//    char   *progname = "initcam" ;
    char    logname[256];
    EdtDev *edt_p = NULL;
    Edtinfo edtinfo;
    char   *unitstr = "0";
    int	    channel = 0 ;
    static char *debug_env = NULL;
    Dependent *dd_p;
    int     pdv_debug = 0;
#ifdef NO_FS
    int     nofs_cfg = 0;
#endif
    int     level;

    /*#ifdef NO_MAIN
    char **argv  = 0 ;
    int argc = 0 ;
    opt_create_argv("initcam",command_line,&argc,&argv);
#endif*/

    *cfgname = '\0';
    *edt_devname = '\0';
    *bitdir = '\0';
    *logname = '\0';


   strcpy(cfgname, fname);
   strcpy(bitdir, bdir);
    /* process arguments 
    --argc;
    ++argv;
    while (argc && argv[0][0] == '-')
    {
	switch (argv[0][1])
	{
	case 'u':		// unit (board) number 
	    ++argv;
	    --argc;
	    if (argc < 1) {
		    fprintf(stderr, "Error: option 'u' requires argument\n");
		    usage(progname);
		    exit(1);
	    }
	    unitstr = argv[0];
	    break;

	case 'c':		// channel number for multi-channel devices 
	    ++argv;
	    --argc;
	    if (argc < 1) {
		    fprintf(stderr, "Error: option 'c' requires argument\n");
		    usage(progname);
		    exit(1);
	    }
	    channel = atoi(argv[0]);
	    break;

	case 'O':		// logfile name 
	    ++argv;
	    --argc;
	    if (argc < 1) {
		    fprintf(stderr, "Error: option 'O' requires file argument\n");
		    usage(progname);
		    exit(1);
	    }
	    strcpy(logname, argv[0]);
	    break;

	case 'q':		// quiet mode 
	    verbose = 0;
	    break;

	case 'v':		// verbose mode 
	    verbose = 1;
	    break;

	case 'V':		// really verbose mode 
	    verbose = 2;
	    break;

	case 'f':		// config filename 
	    ++argv;
	    --argc;
	    if (argc < 1) {
		    fprintf(stderr, "Error: option 'f' requires argument\n");
		    usage(progname);
		    exit(1);
	    }
	    strcpy(cfgname, argv[0]);
#ifdef NO_FS
	    strcpy(bitdir, "_NOFS_");
	    nofs_cfg = 1;
#endif
	    break;

	case 'e':		// no file system embedded bitfile name 
#ifdef NO_FS
	    ++argv;
	    --argc;
	    if (argc < 1) {
		    fprintf(stderr, "Error: option 'e' requires argument\n");
		    usage(progname);
		    exit(1);
	    }
	    strcpy(cfgname, argv[0]);
	    strcpy(bitdir, "_NOFS_");
	    nofs_cfg = 1;
#else
             fprintf(stdout, "\n-e specified but not compiled with nofs configs.\nrecompile with -DNO_FS and try again\n\n");
	     usage(progname);
	     exit(1);
#endif
	    break;

	case 'B':		// don't load bitfile 
	    no_bitload = 1;
	    break;

	case 'b':
	case 'd':		// compat 
	    ++argv;
	    --argc;
	    if (argc < 1) {
		    fprintf(stderr, "Error: options 'b' or 'd' require argument\n");
		    usage(progname);
		    exit(1);
	    }
	    strcpy(bitdir, argv[0]);
	    break;

	case 'h':		// help 
	    usage(progname);
	    exit(0);
	    break;

	case '-':
	    if (strcmp(argv[0], "--help") == 0) {
		    usage(progname);
		    exit(0);
	    } else {
		    fprintf(stderr, "unknown option: %s\n", argv[0]);
		    usage(progname);
		    exit(1);
	    }
	    break;

	default:
	    fprintf(stdout, "unknown flag -'%c'\n", argv[0][1]);
	    usage(progname);
	    exit(1);
	}
	argc--;
	argv++;
    }//while (argc && argv[0][0] == '-')

    if (!(*cfgname))
    {
	usage(progname);
	exit(1);
    }
*/
    /*
     * not using pdv_open to open, but still using pdvlib calls, so force
     * debug if -v or -V or PDVDEBUG envvar
     */
    if (debug_env == NULL
	&& ((debug_env = (char *) getenv("PDVDEBUG")) != NULL)
	&& *debug_env != '0')
	    pdv_debug = atoi(debug_env);

    /*
     * normally the pdvlib error handle gets initialized in pdv_open, but
     * initcam is a special case since it calls edt_open but also uses
     * pdvlib calls -- see edt_msg library.
     */
    level = edt_msg_default_level();

    if ((verbose > 1) && (pdv_debug < verbose))
	pdv_debug = verbose;

    if ((!verbose) && (!pdv_debug))
	level = 0;
    else
    {
	if (verbose > 0)
	{
	    level |= EDTAPP_MSG_INFO_1;
	    level |= PDVLIB_MSG_INFO_1;
	    level |= PDVLIB_MSG_WARNING;
	    level |= PDVLIB_MSG_FATAL;
	}
	if (verbose > 1)
	    level |= EDTAPP_MSG_INFO_2;
	if (pdv_debug > 1)
	    level |= PDVLIB_MSG_INFO_2;
    }

    edt_msg_set_level(edt_msg_default_handle(), level);
    if (*logname)
	edt_msg_set_name(edt_msg_default_handle(), logname);

    /* kind of kludgy... since pdv_setdebug doesn't actually USE edt_p,
     * its okay that it hasn't been opened yet , hence the NULL pointer
     */
    if (pdv_debug)
	pdv_setdebug(NULL, pdv_debug);

    /*
     * if porting this code to an application, be sure to free this 
     * and reallocate if you call pdv_initcam multiple times.
     */
    if ((dd_p = pdv_alloc_dependent()) == NULL)
    {
	edt_msg(PDVLIB_MSG_FATAL, "alloc_dependent FAILED -- exiting\n");
	exit(1);
    }

#ifdef NO_FS
    if (nofs_cfg)
    {
	int ret ; /* randall how should we deal with this? */
	ret = pdv_readcfg_emb(cfgname, dd_p, &edtinfo);
    }
    else
#endif
    if (pdv_readcfg(cfgname, dd_p, &edtinfo) != 0)
    {
	edt_msg(PDVLIB_MSG_FATAL, "readcfg FAILED -- exiting\n");
	exit(1);
    }

    if (no_bitload)
	strcpy(dd_p->rbtfile, "_SKIPPED_");

    /*
     * open the device
     */
    unit = edt_parse_unit_channel(unitstr, edt_devname, "pdv", &channel);
    edt_msg(EDTAPP_MSG_INFO_1, "opening %s unit %d....\n", edt_devname, unit);
    if ((edt_p = edt_open_channel(edt_devname, unit, channel)) == NULL)
    {
	sprintf(errstr, "edt_open(%s%d)", edt_devname, unit);
	edt_perror(errstr);
	return (1);
    }

    if (edt_p->devid == PDVFOI_ID)
    {
#ifdef _FOI_SUPPORTED
	pdv_initcam_set_rci(edt_p, channel) ;
#else
	edt_msg(EDTAPP_MSG_FATAL,"FOI not supported after pkg v4.1.5.9\n");
	edt_close(edt_p);
	exit(1);
#endif
    }

    if (pdv_initcam(edt_p, dd_p, unit, &edtinfo, cfgname, bitdir,
						pdv_debug) != 0)
    {
	edt_msg(EDTAPP_MSG_FATAL,"initcam failed. Run with '-V' to see complete debugging output\n");
	edt_close(edt_p);
	exit(1);
    }

    edt_close(edt_p);
    edt_msg(EDTAPP_MSG_FATAL, "done\n");
#ifdef NO_MAIN
    return(0) ;
#else
    exit(0);
#endif
}



#define edt_swab32(x) \
    (\
    ((u_int)( \
    (((u_int)(x) & (u_int)0x000000ffUL) << 24) | \
    (((u_int)(x) & (u_int)0x0000ff00UL) <<  8) | \
    (((u_int)(x) & (u_int)0x00ff0000UL) >>  8) | \
    (((u_int)(x) & (u_int)0xff000000UL) >> 24) )) \
    )

EdtDev *edt_p;

u_int   tracebuf[EDT_TRACESIZE];

int     curcount;

void    print_hex(u_char *buf, int count);
void    print_ascii(char *buf);


int setdebug(const char * kbuff)
{
    int     i, unit = 0, report = 0, debug = -1, quiet = 0;
    int     channel = 0;
    char   *unitstr = "0";
    char    edt_devname[100];
    int     do_channel = 0;
    int     test_timestamp = 0 ;
    int     get_trace = 0;
    int     wait_trace = 0;
    int     get_sglist = 0;
    int     set_sglist = 0;
    int     set_solaris_dma_mode = 0;
    int     solaris_dma_mode = 0;
    int     set_umem_lock_mode = 0;
    int     umem_lock_mode = 0;
    int     do_timeout = 0;
    int     clock_fifo = 0;
    int     do_loop = 0;

    int     mstimeout = -1;
    int     sgbuf = -1;
    int      version = 0;

    int process_isr = -1;
    int do_process_isr = 0;

    int multi_done = 0x1234;
    int check_openmask = 0 ;

    int use_kernel_buffers = -1;
    int do_kernel_buffers = 0;
    int use_persistent_buffers = 0;

    int max_buffers = 0;
    int set_fv_done = -1;

    int lock_on = 0;
    int lock_on_value = 0;

    u_int set_trace = 0;
    u_int trace_reg = 0;
    u_int trace_state = 0;
    u_int trace_regs_enable = 0;

/*
#ifdef NO_MAIN
    char **argv  = 0 ;
    int argc = 0 ;
    opt_create_argv("setdebug",command_line,&argc,&argv);
#endif
*/

#ifdef P53B
    int     soft_reset = 0;
    int     hard_reset = 0;
    int     set_coupled = 0;
    int     direct_coupled = 0;
    int     arg;

#endif

      
   if (kbuff[0] == 'q')
          use_kernel_buffers = -1;
   else if (kbuff[0] == 'c')
          use_kernel_buffers = EDT_COPY_KBUFS;
   else if (kbuff[0] == 'm')
          use_kernel_buffers = EDT_MMAP_KBUFS;
   else 
          use_kernel_buffers = 0;
      
   if (isdigit(kbuff[0]))
          use_kernel_buffers = strtol(kbuff,0,0);

   if (kbuff[0] && (kbuff[1] == 'p'))
   {
      use_persistent_buffers = 1;
      use_kernel_buffers |= EDT_PERSISTENT_KBUFS;
   }
   do_kernel_buffers = 1;
      
    
/*    --argc;
    ++argv;
    while (argc && argv[0][0] == '-')
    {
   switch (argv[0][1])
   {
   case 'u':
       ++argv;
       --argc;
       unitstr = argv[0];
       break;

   case 'c':
       ++argv;
       --argc;
       channel = atoi(argv[0]);
       do_channel = 1;
       break;

   case 't':
       ++argv;
       --argc;
       mstimeout = atoi(argv[0]);
       do_timeout = 1;
       break;

   case 'i':
       ++argv;
       --argc;
       process_isr = atoi(argv[0]);
       do_process_isr = 1;
       break;

   case 'k':

       if (argc > 1)
       {
      ++argv;
      --argc;
      if (argv[0][0] == 'q')
          use_kernel_buffers = -1;
      else if (argv[0][0] == 'c')
          use_kernel_buffers = EDT_COPY_KBUFS;
      else if (argv[0][0] == 'm')
          use_kernel_buffers = EDT_MMAP_KBUFS;
      else 
          use_kernel_buffers = 0;
      if (isdigit(argv[0][0]))
          use_kernel_buffers = strtol(argv[0],0,0);

      if (argv[0][0] && (argv[0][1] == 'p'))
      {
          use_persistent_buffers = 1;
          use_kernel_buffers |= EDT_PERSISTENT_KBUFS;
      }

       }
       do_kernel_buffers = 1;
       break;


   case 'r':
       report = 1;
       break;

   case 'v':
       version = 1;
       break;

   case 'T':
       set_trace = 1;
            if (argv[0][2] == '+')
      trace_state = 1;
       else if (argv[0][2] == '-')
           trace_state = 0;
       else 
       {
      trace_regs_enable = 1;
       }      
       if (argc > 1)
       {
       ++argv;
       --argc;
       trace_reg = strtol(argv[0],0,0);
       if (!trace_regs_enable)
       {
       if (strlen(argv[0]) < 7 && trace_reg < 0x10000)
       {
           trace_reg |= 0x01010000;
       }
       } 
       }
    
       break;

   case 'l':
       do_loop = 1;
       break;

   case 'L':
       lock_on = 1;
       ++argv;
       --argc;
       lock_on_value = atoi(argv[0]);
       printf("lock_on_value = %d\n", lock_on_value);
       break;

   case 'V':
       ++argv;
       --argc;
       set_fv_done = atoi(argv[0]);
       break;

   case 'd':
       ++argv;
       --argc;
       debug = strtol(argv[0], 0, 0);
       break;

   case 'g':
       get_trace = 1;
       break;

   case 'G':
       get_trace = 1;
       wait_trace = 1;
       break;

#ifdef P53B
   case 'D':
       soft_reset = 1;
       break;

   case 'C':
       set_coupled = 1;
       ++argv;
       --argc;
       direct_coupled = atoi(argv[0]);
       break;

   case 'R':
       hard_reset = 1;
       break;
#else
   case 'f':
       ++argv;
       --argc;
       clock_fifo = atoi(argv[0]);
       break;
   case 's':
       get_sglist = 1;
       ++argv;
       --argc;
       sgbuf = atoi(argv[0]);
       break;

   case 'S':
       set_solaris_dma_mode = 1;
       ++argv;
       --argc;
       solaris_dma_mode = atoi(argv[0]);
       break;

   case 'U':
       set_umem_lock_mode = 1;
       ++argv;
       --argc;
       umem_lock_mode = atoi(argv[0]);
       break;

   case 'b':
       ++argv;
       --argc;
       max_buffers = atoi(argv[0]);
       break;

   case 'm':
       ++argv;
       --argc;
       multi_done = atoi(argv[0]);
       break;

   case 'o':
       check_openmask = 1 ;
       break;

   case 'q':
       quiet = 1;
       break;
   case 'x':
       set_sglist = 1;
       break;
#endif
   default:
#ifdef P53B
       puts(
      "Usage: setdebug [-u unit_no] [-c chan] [-R] [-D] [-r] [-d N] [-i N]");
#else
       puts("Usage: setdebug [-u unit_no] [-c chan] [-r] [-d N] [-i N]");
#endif
       exit(0);
   }
   argc--;
   argv++;
    }*/

    unit = edt_parse_unit(unitstr, edt_devname, EDT_INTERFACE);

    if (do_channel)
    {
   if ((edt_p = edt_open_channel(edt_devname, unit, channel)) == NULL)
   {
       char    str[64];

       sprintf(str, "%s%d_%d", edt_devname, unit, channel);
       perror(str);
       exit(1);
   }
    }
    else if ((edt_p = edt_open(edt_devname, unit)) == NULL)
    {
   char    str[64];

   sprintf(str, "%s%d", edt_devname, unit);
   perror(str);
   exit(1);
    }

    if (trace_regs_enable)
    {
   edt_trace_regs_enable(edt_p, trace_reg);
    } 
    else if (set_trace)
    {
   edt_set_trace_regs(edt_p, trace_reg, trace_state);
    }

    if (check_openmask)
    {
   u_int tmpmask ;
   edt_dma_info tmpinfo ;
   tmpmask = edt_get_dma_info(edt_p, &tmpinfo) ;
   printf("dma info mask %08x used %08x alloc %08x active %08x\n",
       tmpmask,
       tmpinfo.used_dma,
       tmpinfo.alloc_dma,
       tmpinfo.active_dma) ;
   exit(0) ;
    }

    if (lock_on)
    {

   edt_ioctl(edt_p, EDTS_TEST_LOCK_ON, &lock_on_value);

    }

#ifdef P53B
    if (hard_reset)
    {
   arg = 1;
   if (edt_ioctl(edt_p, P53S_RESET, &arg) == -1)
       perror("edt_ioctl(P53S_RESET)");
    }
    if (soft_reset)
    {
   arg = 0;
   if (edt_ioctl(edt_p, P53S_RESET, &arg) == -1)
       perror("edt_ioctl(P53S_RESET)");
    }
    if (set_coupled)
    {
   if (direct_coupled)
       printf("setting direct_coupled\n");
   else
       printf("setting transformer_coupled\n");
   if (edt_ioctl(edt_p, P53S_COUPLED, &direct_coupled) == -1)
       perror("edt_ioctl(P53S_DIRECT)");
    }
#endif            /* P53B */


    if (do_timeout)
    {
   printf("setting read timeout to %d ms\n", mstimeout);
   edt_set_rtimeout(edt_p, mstimeout);
   edt_set_wtimeout(edt_p, mstimeout);
    }

    if (do_process_isr)
    {
   edt_ioctl(edt_p, EDTS_PROCESS_ISR, &process_isr);
   printf("Process ISR state = %d\n", process_isr);

    }

    if (multi_done != 0x1234)
    {

   if (multi_done < 0)
   {
       edt_ioctl(edt_p, EDTG_MULTI_DONE,&multi_done);
       printf("multi done = %d\n", multi_done);
   } else {
       printf("setting multi done = %d\n", multi_done);
       edt_ioctl(edt_p, EDTS_MULTI_DONE, &multi_done);
   }
    }

    if (do_kernel_buffers)
    {
   int old_state;

   if (use_kernel_buffers != -1)
   {
       printf("Setting to %d\n",
      use_kernel_buffers);
       old_state = edt_set_kernel_buffers(edt_p,use_kernel_buffers);
       printf("old_state = %d\n", old_state);
   }

   old_state = edt_get_kernel_buffers(edt_p);
   use_persistent_buffers = edt_get_persistent_buffers(edt_p);

   printf("Use Kernel buffers is %s %s\n", (old_state == 0)?
       "off":(old_state == 2)?"mmapped":"copy",
       (use_persistent_buffers)?"(persistent)":"");

    }

    if (max_buffers > 0)
    {
   edt_set_max_buffers(edt_p,max_buffers);
   printf("Max Buffers = %d\n", edt_get_max_buffers(edt_p));

    }

    if (version)
    {

   edt_version_string version;
   edt_version_string build;

   edt_get_driver_version(edt_p,version,sizeof(version));
   edt_get_driver_buildid(edt_p,build,sizeof(version));

   printf("\n%s%d:  Driver version %s \n       Build %s\n", 
       edt_devname, 
       unit, 
       version,
       build);
    }

    if (report)
    {

   i = edt_get_debug(edt_p);

   printf("\n%s%d:  current debug %d type %d\n", edt_devname, unit, i,
       edt_get_drivertype(edt_p));
    }
    if (test_timestamp)
    {
   for(i = 0 ; i < 4 ; i++)
   {
       edt_ref_tmstamp(edt_p,i) ;
       edt_msleep (1000) ;
   }
    }

    if (set_fv_done != -1)
    {
   int i;
   i = (set_fv_done)?1:0;
   edt_ioctl(edt_p,EDTS_FVAL_DONE,&i);
   if (set_fv_done == 1)
       edt_reg_and(edt_p,PDV_UTIL3,~PDV_TRIGINT);
    } 

    if (debug != -1)
    {
   (void) edt_set_debug(edt_p, debug);
   if (report)
       printf("%s%d:  new debug     %d\n", edt_devname, unit, debug);

   if (!quiet)
       edt_dump_registers(edt_p, debug);

   printf("xfer %d bytes\n", edt_get_bytecount(edt_p));

   {
       u_int mapsize;

       mapsize = edt_get_mappable_size(edt_p);

       if (mapsize != 0)
       {
      printf("Second memory space size = 0x%x bytes\n", mapsize);

       }
   }
    }

    if (get_trace)
    {
   edt_trace(edt_p, stdout, FALSE, (wait_trace)?0:1, NULL, 0);

    }
    if (get_sglist)
    {
   edt_buf sg_args;
   u_int   todo_size;
   u_int   sg_size;
   u_int  *todo_list;
   u_int  *sg_list;
   u_int  *log_list;
   u_int   sg_virtual;
   u_int   sg_physical;
   u_int   todo_virtual;
   u_int   first_sg;
   u_int  *ptr;
   u_int   ii;

   sg_args.desc = EDT_SGLIST_SIZE;
   sg_args.value = sgbuf;
   edt_ioctl(edt_p, EDTG_SGINFO, &sg_args);
   sg_size = (u_int) sg_args.value;

   sg_args.desc = EDT_SGLIST_VIRTUAL;
   sg_args.value = sgbuf;
   edt_ioctl(edt_p, EDTG_SGINFO, &sg_args);
   sg_virtual = (u_int) sg_args.value;

   sg_args.desc = EDT_SGLIST_PHYSICAL;
   sg_args.value = sgbuf;
   edt_ioctl(edt_p, EDTG_SGINFO, &sg_args);
   sg_physical = (u_int) sg_args.value;

   sg_args.desc = EDT_SGTODO_SIZE;
   sg_args.value = sgbuf;
   edt_ioctl(edt_p, EDTG_SGINFO, &sg_args);
   todo_size = (u_int) sg_args.value;

   sg_args.desc = EDT_SGTODO_VIRTUAL;
   sg_args.value = sgbuf;
   edt_ioctl(edt_p, EDTG_SGINFO, &sg_args);
   todo_virtual = (u_int) sg_args.value;

   sg_args.desc = EDT_SGTODO_FIRST_SG;
   sg_args.value = sgbuf;
   edt_ioctl(edt_p, EDTG_SGINFO, &sg_args);
   first_sg = (u_int) sg_args.value;

   printf("todo size 0x%x addr  %x\n", todo_size, todo_virtual);
   printf("sg size 0x%x viraddr %x physaddr %x first_sg %x\n",
       sg_size, sg_virtual, sg_physical, first_sg);


   todo_list = (u_int *) malloc(todo_size);
   sg_list = (u_int *) malloc(sg_size);

   if (todo_size)
   {
       printf("return to read todo list: ");
       getchar();
       printf("todo list %p\n", todo_list);
       edt_ioctl(edt_p, EDTG_SGTODO, todo_list);
       ptr = todo_list;
       printf("todo_list:\n");
       printf("todo size %x\n", todo_size);
       printf("return to continue");
       getchar();
       for (ii = 0; ii < todo_size / 8; ii++)
       {
      printf("%08x %08x\n", *ptr, *(ptr + 1));
      ptr += 2;
       }
   }
   if (sg_size)
   {
       buf_args sg_getargs ;
       sg_getargs.addr = (uint64_t) ((unsigned long) sg_list) ;
       sg_getargs.size = sg_size ;
       sg_getargs.index = sgbuf ;
       printf("return to read sg list: %x at %p",sg_size,sg_list);
       getchar();
       edt_ioctl(edt_p, EDTG_SGLIST, &sg_getargs);
       ptr = sg_list;
       printf("sg list:\n");
       if (edt_little_endian())
      for (ii = 0; ii < sg_size / 8; ii++)
      {
          printf("%08x %08x\n", *ptr, *(ptr + 1));
          ptr += 2;
      }
       else
      for (ii = 0; ii < sg_size / 8; ii++)
      {
          printf("%08x %08x\n", edt_swab32(*ptr), edt_swab32(*(ptr + 1)));
          ptr += 2;
      }
   }
   if (set_sglist)
   {

       printf("return to test sg list: ");
       getchar();
       log_list = (u_int *) malloc(1024 * 8);
       for (i = 7; i >= 0; i--)
       {
      log_list[i * 2] = 1024 * (1024 / 8) * i ;
      log_list[i * 2 + 1] = 1024 * (1024 / 8) ;
       }
       edt_set_sglist(edt_p, 0, log_list, 8) ;
   }
    }
    if (clock_fifo)
    {
   /* tmp test code for ssdio(1) and pcd(2) */
   for (;;)
   {
       if (clock_fifo == 1)
       {
      u_int   stat;
      u_int   tmp;
      u_int   bytenum;

      stat = edt_reg_read(edt_p, PCD_STAT);
      bytenum = (stat & (SSDIO_BYTECNT_MSK)) >> SSDIO_BYTECNT_SHFT;
      printf("stat %02x next strobe ", stat);
      if (stat & SSDIO_LAST_BIT)
          printf("will fill byte to start filling %d\n", bytenum);
      else
          printf("filling byte %d\n", bytenum);
      tmp = edt_reg_read(edt_p, PCD_FUNCT);
      printf("return to strobe: ");
      getchar();
      edt_reg_write(edt_p, PCD_FUNCT, tmp & ~SSDIO_STROBE);
      edt_reg_write(edt_p, PCD_FUNCT, tmp | SSDIO_STROBE);
       }
       else
       {
      u_int   cfg;
      u_int   cnt;

      cnt = edt_get_bytecount(edt_p);
      cfg = edt_reg_read(edt_p, EDT_DMA_CFG);
      printf("0x%x tranferred  fifo %d cfg %08x\n",
          cnt, (cfg & EDT_FIFO_CNT) >> EDT_FIFO_SHIFT, cfg);
      printf("return to clock fifo: ");
      getchar();
      edt_reg_write(edt_p, EDT_DMA_INTCFG, cfg | EDT_WRITE_STROBE);
       }
   }
    }
    if (do_loop)
    {
   u_int   bcount;
   u_int buf ;

   for (;;)
   {
       edt_msleep(1000);
       bcount = edt_get_bytecount(edt_p);
       printf("xfer %d (0x%x) bytes\n", bcount, bcount);
       bcount = edt_get_bufbytecount(edt_p,&buf);
       printf("xfer %d (0x%x) bytes buf %x\n", bcount, bcount,buf);
   }
    }
    if (set_solaris_dma_mode)
    {
   edt_ioctl(edt_p, EDTS_SOLARIS_DMA_MODE, &solaris_dma_mode);
    }
    if (set_umem_lock_mode)
    {
   edt_ioctl(edt_p, EDTS_UMEM_LOCK, &umem_lock_mode);
    }


#ifndef NO_MAIN
    exit(0);
#endif
    return (0);
}


void
print_hex(u_char *buf, int count)
{
    int i;

    printf("<");
    for (i=0; i<count; i++)
   printf("%02x%s", buf[i], i == count-1?">":" ");
    printf("\n");
}

void
print_ascii(char *buf)
{
    unsigned int i;

    printf("<");
    for (i=0; i<strlen(buf); i++)
    {
   if (buf[i] >= ' ' && buf[i] <= '~')
       printf("%c", buf[i]);
   else if (buf[i] == '\t')
       printf("\\t");
   else if (buf[i] == '\n')
       printf("\\n");
   else if (buf[i] == '\r')
       printf("\\r");
    }
    printf(">\n");
}

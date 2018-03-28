#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#include "core_allvars.h"
#include "core_proto.h"

#ifdef HDF5
#include <hdf5.h>

hid_t hdf_file;
#endif

// keep a static file handle to remove the need to do constant seeking
FILE* load_fd = NULL;


void load_tree_table(int filenr)
{
  if (strncmp(FileType, "binary", 512) == 0)
  {
    load_tree_table_binary(filenr);
  }
  else
  {
    load_tree_table_hdf(filenr);
  }

}

void load_tree_table_binary(int filenr)
{
  int i, n, totNHalos;
  char buf[1000];
  FILE *fd;

	// open the file each time this function is called
  sprintf(buf, "%s/%s.%d%s", SimulationDir, TreeName, filenr, TreeExtension);
  if(!(load_fd = fopen(buf, "r")))
  {
    printf("can't open file `%s'\n", buf);
    ABORT(0);
  }

  myfread(&Ntrees, 1, sizeof(int), load_fd);
  myfread(&totNHalos, 1, sizeof(int), load_fd);

  TreeNHalos = mymalloc(sizeof(int) * Ntrees);
  TreeFirstHalo = mymalloc(sizeof(int) * Ntrees);

  for(n = 0; n < NOUT; n++)
    TreeNgals[n] = mymalloc(sizeof(int) * Ntrees);
  myfread(TreeNHalos, Ntrees, sizeof(int), load_fd);

  if(Ntrees)
    TreeFirstHalo[0] = 0;
  for(i = 1; i < Ntrees; i++)
    TreeFirstHalo[i] = TreeFirstHalo[i - 1] + TreeNHalos[i - 1];

  for(n = 0; n < NOUT; n++)
  {
    for(i = 0; i < Ntrees; i++)
      TreeNgals[n][i] = 0;

    sprintf(buf, "%s/%s_z%1.3f_%d", OutputDir, FileNameGalaxies, ZZ[ListOutputSnaps[n]], filenr);

    if(!(fd = fopen(buf, "w")))
    {
      printf("can't open file `%s'\n", buf);
      ABORT(0);
    }
    fclose(fd);
    TotGalaxies[n] = 0;
  }
}

#ifdef HDF5
void load_tree_table_hdf(int filenr)
{

  char buf[1000];
  int32_t totNHalos, i;
  int32_t status;

  sprintf(buf, "%s/%s.%d%s", SimulationDir, TreeName, filenr, TreeExtension);
  hdf_file = H5Fopen(buf, H5F_ACC_RDONLY, H5P_DEFAULT);
  if (hdf_file < 0)
  {
    printf("can't open file `%s'\n", buf);
    ABORT(0);    
  }

  /*
  struct METADATA_NAMES 
  {
    char name_NTrees[1024];
    char name_totNHalos[1024];
    char name_TreeNHalos[1024];
  }; 

  struct METADATA_NAMES *metadata_names;

  snprintf(metadata_names->name_NTrees, 1023, "Ntrees");
  snprintf(metadata_names->name_totNHalos, 1023, "totNHalos");
  snprintf(metadata_names->name_totNHalos, 1023, "TreeNHalos");
  */ 

  status = read_attribute_int(hdf_file, "/Header", "Ntrees", &Ntrees);
  if (status == EXIT_FAILURE)
  {
    fprintf(stderr, "Error while processing file %s\n", buf);
    ABORT(0);
  }

  status = read_attribute_int(hdf_file, "/Header", "totNHalos", &totNHalos);
  if (status == EXIT_FAILURE)
  {
    fprintf(stderr, "Error while processing file %s\n", buf);
    ABORT(0);
  }

  printf("There are %d trees and %d total halos\n", Ntrees, totNHalos);
  
  TreeNHalos = mymalloc(sizeof(int) * Ntrees); 

  status = read_attribute_int(hdf_file, "/Header", "TreeNHalos", TreeNHalos);
  if (status == EXIT_FAILURE)
  {
    fprintf(stderr, "Error while processing file %s\n", buf);
    ABORT(0);
  }

  TreeFirstHalo = mymalloc(sizeof(int) * Ntrees);

  for (i = 0; i < 20; ++i)
    printf("Tree %d: NHalos %d\n", i, TreeNHalos[i]);

  if(Ntrees)
    TreeFirstHalo[0] = 0;
  for(i = 1; i < Ntrees; i++)
    TreeFirstHalo[i] = TreeFirstHalo[i - 1] + TreeNHalos[i - 1];
 exit(0); 
}

int32_t read_attribute_int(hid_t hdf_file, char *groupname, char *attr_name, int *attribute)
{

  herr_t status;
  hid_t attr_id; 

  attr_id = H5Aopen_by_name(hdf_file, groupname, attr_name, H5P_DEFAULT, H5P_DEFAULT);
  if (attr_id < 0.0)
  {
    fprintf(stderr, "Could not open the attribute %s in group %s\n", attr_name, groupname);
    return EXIT_FAILURE;
  }

  status = H5Aread(attr_id, H5T_NATIVE_INT, attribute);
  if (status < 0.0)
  {
    fprintf(stderr, "Could not read the attribute %s in group %s\n", attr_name, groupname);
    return EXIT_FAILURE;
  }
    
  status = H5Aclose(attr_id);
  if (status < 0.0)
  {
    fprintf(stderr, "Error when closing the file.\n"); 
    return EXIT_FAILURE;
  }
   
  return EXIT_SUCCESS; 
} 

#endif
void free_tree_table(void)
{
  int n;

  for(n = NOUT - 1; n >= 0; n--)
    myfree(TreeNgals[n]);

  myfree(TreeFirstHalo);
  myfree(TreeNHalos);
	
	// Don't forget to free the open file handle
	if(load_fd) {
		fclose(load_fd);
		load_fd = NULL;
	}
}



void load_tree(int filenr, int nr)
{
  int i;

  // must have an FD
  assert( load_fd );

  Halo = mymalloc(sizeof(struct halo_data) * TreeNHalos[nr]);

  myfread(Halo, TreeNHalos[nr], sizeof(struct halo_data), load_fd);

  MaxGals = (int)(MAXGALFAC * TreeNHalos[nr]);
  if(MaxGals < 10000)
    MaxGals = 10000;

  FoF_MaxGals = 10000;

  HaloAux = mymalloc(sizeof(struct halo_aux_data) * TreeNHalos[nr]);
  HaloGal = mymalloc(sizeof(struct GALAXY) * MaxGals);
  Gal = mymalloc(sizeof(struct GALAXY) * FoF_MaxGals);

  for(i = 0; i < TreeNHalos[nr]; i++)
  {
    HaloAux[i].DoneFlag = 0;
    HaloAux[i].HaloFlag = 0;
    HaloAux[i].NGalaxies = 0;
  }

}



void free_galaxies_and_tree(void)
{
  myfree(Gal);
  myfree(HaloGal);
  myfree(HaloAux);
  myfree(Halo);
}



size_t myfread(void *ptr, size_t size, size_t nmemb, FILE * stream)
{
  return fread(ptr, size, nmemb, stream);
}

size_t myfwrite(void *ptr, size_t size, size_t nmemb, FILE * stream)
{
  return fwrite(ptr, size, nmemb, stream);
}

int myfseek(FILE * stream, long offset, int whence)
{
  return fseek(stream, offset, whence);
}

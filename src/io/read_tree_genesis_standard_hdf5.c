#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "read_tree_genesis_standard_hdf5.h"
#include "../core_mymalloc.h"


/* Local Structs */
struct METADATA_NAMES 
{
  char name_NTrees[MAX_STRING_LEN];
  char name_totNHalos[MAX_STRING_LEN];
  char name_TreeNHalos[MAX_STRING_LEN];
}; 


enum hdf5_dtype
{
    H5_INT32 = 0,
    H5_INT64,
    H5_REAL32,
    H5_REAL64,
}


/* Local Proto-Types */
/* static int32_t fill_genesis_metadata_names(struct METADATA_NAMES *metadata_names, enum Valid_TreeTypes my_TreeType); */
static int32_t read_attribute_int(hid_t my_hdf5_file, char *groupname, char *attr_name, int *attribute);
static int32_t read_dataset(char *dataset_name, int32_t datatype, void *buffer, struct forest_info *forests_info);

/* Externally visible Functions */
void load_forest_table_genesis_hdf5(struct forest_info *forests_info)
{
    int32_t totNHalos;
    int32_t status;

    struct METADATA_NAMES metadata_names;
    const char *filename = forests_info->filename;
    forests_info->hdf5_fp = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);

    if (forests_info->hdf5_fp < 0) {
        fprintf(stderr,"can't open file `%s'\n", filename);
        ABORT(FILE_NOT_FOUND);
    }

    /* status = fill_genesis_metadata_names(&metadata_names, run_params.TreeType); */
    /* if (status != EXIT_SUCCESS) { */
    /*     ABORT(status); */
    /* } */
 
    //read attribute -> hf['Header']['Particle_mass'].attrs['DarkMatter']
    //read attribute -> hf['Header'].attrs['NSnaps']

    status = read_attribute_int(forests_info->hdf5_fp, "/Header", "NSnaps", &(forests_info->nsnapshots));
    if (status != EXIT_SUCCESS) {
        fprintf(stderr, "Error while processing file %s\n", filename);
        fprintf(stderr, "Error code is %d\n", status);
        perror(NULL);
        ABORT(FILE_READ_ERROR);
    }


    /* const int nforests = forests_info->nforests; */
    
    /* status = read_attribute_int(forests_info->hdf5_fp, "/Header", metadata_names.name_totNHalos, &totNHalos); */
    /* if (status != EXIT_SUCCESS) {  */
    /*     fprintf(stderr, "Error while processing file %s\n", filename); */
    /*     fprintf(stderr, "Error code is %d\n", status); */
    /*     ABORT(0); */
    /* } */

  
    /* forests_info->totnhalos_per_forest = mymalloc(sizeof(int) * nforests);  */
    /* int *forestnhalos = forests_info->totnhalos_per_forest; */
    
    /* status = read_attribute_int(forests_info->hdf5_fp, "/Header", metadata_names.name_TreeNHalos, forestnhalos); */
    /* if (status != EXIT_SUCCESS) { */
    /*     fprintf(stderr, "Error while processing file %s\n", filename); */
    /*     fprintf(stderr, "Error code is %d\n", status); */
    /*     ABORT(0); */
    /* } */
    
    


    
}

#define READ_GENESIS_TREE_PROPERTY(nsnaps, sage_name, hdf5_name, type_int, data_type) \
    {                                                                   \
        snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/%s", forestnr, #hdf5_name); \
        status = read_dataset(dataset_name, type_int, buffer, forests_info);         \
        if (status != EXIT_SUCCESS) {                                   \
            ABORT(0);                                                   \
        }                                                               \
        for (int halo_idx = 0; halo_idx < nhalos; ++halo_idx) {         \
            local_halos[halo_idx].sage_name = ((data_type*)buffer)[halo_idx];  \
        }                                                               \
    }                                                                   \

#define READ_GENESIS_TREE_PROPERTY_MULTIPLEDIM(nsnaps, sage_name, hdf5_name, type_int, data_type) \
    {                                                                   \
        snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/%s", forestnr, #hdf5_name); \
        status = read_dataset(dataset_name, type_int, buffer_multipledim, forests_info); \
        if (status != EXIT_SUCCESS) {                                   \
            ABORT(0);                                                   \
        }                                                               \
        for (int halo_idx = 0; halo_idx < nhalos; ++halo_idx) { \
            for (int dim = 0; dim < NDIM; ++dim) {                      \
                local_halos[halo_idx].sage_name[dim] = ((data_type*)buffer_multipledim)[halo_idx * NDIM + dim]; \
            }                                                           \
        }                                                               \
    }                                                                   \

/*
  Fields in the particle data type, stored at each snapshot
  ['Efrac', 'ForestID', 'ForestLevel',
  'Head', 'HeadRank', 'HeadSnap',
  'ID',
  'Lx', 'Ly', 'Lz',
  'Mass_200crit', 'Mass_200mean', 'Mass_FOF', 'Mass_tot',
  'Num_descen', 'Num_progen',
  'RVmax_Lx', 'RVmax_Ly', 'RVmax_Lz',
  'RVmax_sigV', 'R_200crit', 'R_200mean',
  'R_HalfMass', 'R_size', 'Rmax',
  'RootHead', 'RootHeadSnap', 'RootTail', 'RootTailSnap',
  'Structuretype',
  'Tail', 'TailSnap',
  'VXc', 'VYc', 'VZc', 'Vmax',
  'Xc', 'Yc', 'Zc',
  'cNFW',
  'hostHaloID',
  'lambda_B',
  'npart',
  'numSubStruct',
  'sigV']


----------------------------  
  From the ASTRO 3D wiki, here is info about the fields.
  
  This format as several key fields per snapshot:

  Head: A halo ID pointing the immediate descendant of a halo. With temporally unique ids, this id encodes both the snapshot that the descendant is at and the index in the properties array
  HeadSnap: The snapshot of the immediate descendant
  RootHead: Final descendant
  RootHeadSnap: Final descendant snapshot
  Tail: A halo ID pointing to the immediate progenitor
  TailSnap, RootTail, RootTailSnap: similar in operation to HeadSnap, RootHead, RootHeadSnap but for progenitors
  ID: The halo ID
  Num_progen: number of progenitors
  
  There are also additional fields that are present for Meraxes,
  
  ForestID: A unique id that groups all descendants of a field halo and any subhalos it may have contained (which can link halos together if one was initially a subhalo of the other).This is computationally intensive. Allows for quick parsing of all halos to identify those that interact across cosmic time.
  
  To walk the tree, one needs only to move forward/backward in time one just needs to get Head or Tail and access the data given by that ID.
  The temporally unique ID is given by:
  
  ID = snapshot*1e12 + halo index
  
----------------------------

 */


void load_forest_genesis_hdf5(int32_t forestnr, const int32_t nhalos, struct halo_data **halos, struct forest_info *forests_info)
{
    char dataset_name[MAX_STRING_LEN];
    int32_t status;

    double *buffer; // Buffer to hold the read HDF5 data.  
    // The largest data-type will be double. 

    double *buffer_multipledim; // However also need a buffer three times as large to hold data such as position/velocity.

    if (forests_info->hdf5_fp <= 0) {
        fprintf(stderr, "The HDF5 file should still be opened when reading the halos in the forest.\n");
        fprintf(stderr, "For forest %d we encountered error\n", forestnr);
        H5Eprint(forests_info->hdf5_fp, stderr);
        ABORT(NULL_POINTER_FOUND);
    }


    *halos = mymalloc(sizeof(struct halo_data) * nhalos); 
    struct halo_data *local_halos = *halos;
    
    buffer = calloc(nhalos, sizeof(*(buffer)));
    if (buffer == NULL) {
        fprintf(stderr, "Could not allocate memory for the HDF5 buffer.\n");
        ABORT(MALLOC_FAILURE);
    }

    buffer_multipledim = calloc(nhalos * NDIM, sizeof(*(buffer_multipledim))); 
    if (buffer_multipledim == NULL) {
        fprintf(stderr, "Could not allocate memory for the HDF5 multiple dimension buffer.\n");
        ABORT(MALLOC_FAILURE);
    }

    // We now need to read in all the halo fields for this forest.
    // To do so, we read the field into a buffer and then properly slot the field into the Halo struct.

    /* Merger Tree Pointers */ 
    READ_GENESIS_TREE_PROPERTY(MaxSnapshot, Descendant, Descendant, H5_INT32, int);
    READ_GENESIS_TREE_PROPERTY(MaxSnapshot, FirstProgenitor, FirstProgenitor, H5_INT32, int);
    READ_GENESIS_TREE_PROPERTY(MaxSnapshot, NextProgenitor, NextProgenitor, H5_INT32, int);
    READ_GENESIS_TREE_PROPERTY(MaxSnapshot, FirstHaloInFOFgroup, FirstHaloInFOFgroup, H5_INT32, int);
    READ_GENESIS_TREE_PROPERTY(MaxSnapshot, NextHaloInFOFgroup, NextHaloInFOFgroup, H5_INT32, int);

    /* Halo Properties */ 
    READ_GENESIS_TREE_PROPERTY(MaxSnapshot, Len, Len, H5_INT32, int);
    READ_GENESIS_TREE_PROPERTY(MaxSnapshot, M_Mean200, M_mean200, H5_REAL32, FLOAT);
    READ_GENESIS_TREE_PROPERTY(MaxSnapshot, Mvir, Mvir, H5_REAL32, float);
    READ_GENESIS_TREE_PROPERTY(MaxSnapshot, M_TopHat, M_TopHat, H5_REAL32, float);
    READ_GENESIS_TREE_PROPERTY_MULTIPLEDIM(MaxSnapshot, Pos, Pos, H5_REAL32, float);
    READ_GENESIS_TREE_PROPERTY_MULTIPLEDIM(MaxSnapshot, Vel, Vel, H5_REAL32, float);
    READ_GENESIS_TREE_PROPERTY(MaxSnapshot, VelDisp, VelDisp, H5_REAL32, float);
    READ_GENESIS_TREE_PROPERTY(MaxSnapshot, Vmax, Vmax, H5_REAL32, float);
    READ_GENESIS_TREE_PROPERTY_MULTIPLEDIM(MaxSnapshot, Spin, Spin, H5_REAL32, float);
    READ_GENESIS_TREE_PROPERTY(MaxSnapshot, MostBoundID, MostBoundID, H5_INT64, long long);

    /* File Position Info */
    READ_GENESIS_TREE_PROPERTY(MaxSnapshot, SnapNum, SnapNum, H5_INT32, int);
    READ_GENESIS_TREE_PROPERTY(MaxSnapshot, FileNr, Filenr, H5_INT32, int);
    READ_GENESIS_TREE_PROPERTY(MaxSnapshot, SubhaloIndex, SubHaloIndex, H5_INT32, int);
    READ_GENESIS_TREE_PROPERTY(MaxSnapshot, SubHalfMass, SubHalfMass, H5_INT32, int);

    free(buffer);
    free(buffer_multipledim);

#ifdef DEBUG_HDF5_READER 
    for (int32_t i = 0; i < 20; ++i) {
        printf("halo %d: Descendant %d FirstProg %d x %.4f y %.4f z %.4f\n", i, local_halos[i].Descendant, local_halos[i].FirstProgenitor, local_halos[i].Pos[0], local_halos[i].Pos[1], local_halos[i].Pos[2]); 
    }
    ABORT(0);
#endif 
 
}

#undef READ_GENESIS_TREE_PROPERTY
#undef READ_GENESIS_TREE_PROPERTY_MULTIPLEDIM

void close_genesis_hdf5_file(struct forest_info *forests_info)
{

    H5Fclose(forests_info->hdf5_fp);

}

/* Local Functions */
static int32_t read_attribute_int(hid_t my_hdf5_file, char *groupname, char *attr_name, int *attribute)
{

    int32_t status;
    hid_t attr_id; 

    attr_id = H5Aopen_by_name(my_hdf5_file, groupname, attr_name, H5P_DEFAULT, H5P_DEFAULT);
    if (attr_id < 0) {
        fprintf(stderr, "Could not open the attribute %s in group %s\n", attr_name, groupname);
        return attr_id;
    }

    status = H5Aread(attr_id, H5T_NATIVE_INT, attribute);
    if (status < 0) {
        fprintf(stderr, "Could not read the attribute %s in group %s\n", attr_name, groupname);
        return status;
    }
    
    status = H5Aclose(attr_id);
    if (status < 0) {
        fprintf(stderr, "Error when closing the file.\n"); 
        H5Eprint(status, stderr);
        return status;
    }
   
    return EXIT_SUCCESS; 
}

static int32_t read_dataset(char *dataset_name, int32_t datatype, void *buffer, struct forest_info *forests_info)
{
    hid_t dataset_id;

    dataset_id = H5Dopen2(forests_info->hdf5_fp, dataset_name, H5P_DEFAULT);
    if (dataset_id < 0) {
        fprintf(stderr, "Error encountered when trying to open up dataset %s\n", dataset_name); 
        H5Eprint(dataset_id, stderr);
        return dataset_id;
    }

    switch (datatype)
        {

        case H5_INT32:
            H5Dread(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);  
            break;

        case H5_REAL32:
            H5Dread(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);
            break;
        case H5_INT64:
            H5Dread(dataset_id, H5T_NATIVE_LLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);            
            break;
            
        default:
            fprintf(stderr, "Your data type has not been included in the switch statement for ``%s`` in file ``%s``.\n", __FUNCTION__, __FILE__);
            fprintf(stderr, "Please add it there.\n");
            ABORT(EXIT_FAILURE);

        }
  
    return EXIT_SUCCESS;
}

/*===========================================================================
=                                                                           =
=                       MtkFileGridToNativeFieldList                        =
=                                                                           =
=============================================================================

                         Jet Propulsion Laboratory
                                   MISR
                               MISR Toolkit

            Copyright 2005, California Institute of Technology.
                           ALL RIGHTS RESERVED.
                 U.S. Government Sponsorship acknowledged.

============================================================================*/

#include "MisrFileQuery.h"
#include "MisrUtil.h"
#include "MisrError.h"
#include <hdf.h>
#include <HdfEosDef.h>
#include <string.h>

/** \brief Read list of native fields from file (excludes derived fields)
 *
 *  \return MTK_SUCCESS if successful.
 *
 *  \par Example:
 *  In this example, we read the list of native fields (excludes derived fields) from the file
 *  \c MISR_AM1_GRP_TERRAIN_GM_P161_O012115_DF_F03_0021.hdf and the grid \c BlueBand
 *
 *  \code
 *  status = MtkFileGridToNativeFieldList("MISR_AM1_GRP_TERRAIN_GM_P161_O012115_DF_F03_0021.hdf", "BlueBand", &nfields, &fieldlist);
 *  \endcode
 *
 *  \note
 *  The caller is responsible for using MtkStringListFree() to free the memory used by \a fieldlist
 */

MTKt_status MtkFileGridToNativeFieldList(
  const char *filename, /**< [IN] Filename */
  const char *gridname, /**< [IN] Gridname */
  int *nfields, /**< [OUT] Number of Fields */
  char **fieldlist[] /**< [OUT] List of Fields */ )
{
  MTKt_status status;		/* Return status of called functions */
  MTKt_status status_code;	/* Return status of this function. */
  intn hdfstatus;		/* HDF-EOS return status */
  int32 Fid = FAIL;             /* HDF-EOS File ID */

  if (filename == NULL)
    MTK_ERR_CODE_JUMP(MTK_NULLPTR);  

  /* Open HDF file for reading */
  hdfstatus = Fid = GDopen((char*)filename,DFACC_READ);
  if (hdfstatus == FAIL)
    MTK_ERR_CODE_JUMP(MTK_HDFEOS_GDOPEN_FAILED);

  /* Read list of fields. */
  status = MtkFileGridToNativeFieldListFid(Fid, gridname, nfields, fieldlist);
  MTK_ERR_COND_JUMP(status);

  /* Close file. */
  hdfstatus = GDclose(Fid);
  if (hdfstatus == FAIL)
    MTK_ERR_CODE_JUMP(MTK_HDFEOS_GDCLOSE_FAILED);
  Fid = FAIL;

  return MTK_SUCCESS;

 ERROR_HANDLE:
  if (Fid != FAIL) GDclose(Fid);

  return status_code;
}

/** \brief Version of MtkFileGridToNativeFieldList that takes an HDF-EOS file identifier rather than a filename.
 *
 *  \return MTK_SUCCESS if successful.
 */

MTKt_status MtkFileGridToNativeFieldListFid(
  int32 Fid,            /**< [IN] HDF-EOS file identifier */
  const char *gridname, /**< [IN] Gridname */
  int *nfields, /**< [OUT] Number of Fields */
  char **fieldlist[] /**< [OUT] List of Fields */ )
{
  MTKt_status status_code;	/* Return status of this function. */
  intn hdfstatus;		/* HDF-EOS return status */
  int32 Gid = FAIL;             /* HDF-EOS Grid ID */
  int32 num_fields = 0;         /* Number of fields */
  char *list = NULL;            /* List of fields */
  int i;
  char *temp = NULL;
  int32 str_buffer_size = 0;

  /* Check Arguments */
  if (fieldlist == NULL)
    MTK_ERR_CODE_JUMP(MTK_NULLPTR);

  *fieldlist = NULL; /* Set output to NULL to prevent freeing unallocated
                        memory in case of error. */

  if (gridname == NULL)
    MTK_ERR_CODE_JUMP(MTK_NULLPTR);

  if (nfields == NULL)
    MTK_ERR_CODE_JUMP(MTK_NULLPTR);

  /* Attach to grid */
  hdfstatus = Gid = GDattach(Fid,(char*)gridname);
  if (hdfstatus == FAIL)
    MTK_ERR_CODE_JUMP(MTK_HDFEOS_GDATTACH_FAILED);

  /* Query length of fields string */
  hdfstatus = GDnentries(Gid, HDFE_NENTDFLD, &str_buffer_size);
  if(hdfstatus == FAIL)
    MTK_ERR_CODE_JUMP(MTK_HDFEOS_GDNENTRIES_FAILED);

  list = (char*)malloc((str_buffer_size + 1)  * sizeof(char));
  if (list == NULL)
    MTK_ERR_CODE_JUMP(MTK_MALLOC_FAILED);

  /* Get list of fields */
  hdfstatus = num_fields = GDinqfields(Gid,list,NULL,NULL);
  if (hdfstatus == FAIL)
    MTK_ERR_CODE_JUMP(MTK_HDFEOS_GDINQFIELDS_FAILED);

  *nfields = num_fields;
  *fieldlist = (char**)calloc(num_fields,sizeof(char*));
  if (*fieldlist == NULL)
    MTK_ERR_CODE_JUMP(MTK_CALLOC_FAILED);
    
  temp = strtok(list,",");
  i = 0;
  while (temp != NULL)
  {
    (*fieldlist)[i] = (char*)malloc((strlen(temp) + 1) * sizeof(char));
    if ((*fieldlist)[i] == NULL)
      MTK_ERR_CODE_JUMP(MTK_MALLOC_FAILED);
    strcpy((*fieldlist)[i],temp);
    temp = strtok(NULL,",");
    ++i;
  }

  free(list);

  hdfstatus = GDdetach(Gid);
  if (hdfstatus == FAIL)
    MTK_ERR_CODE_JUMP(MTK_HDFEOS_GDDETACH_FAILED);

  return MTK_SUCCESS;

 ERROR_HANDLE:
  if (fieldlist != NULL)
    MtkStringListFree(num_fields, fieldlist);

  if (nfields != NULL)
    *nfields = -1;

  free(list);
  GDdetach(Gid);

  return status_code;
}

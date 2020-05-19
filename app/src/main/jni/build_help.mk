# https://android.googlesource.com/platform/build/+/master/core/definitions.mk

###########################################################
# Use utility find to find given files in the given subdirs.
# This function uses $(1), instead of LOCAL_PATH as the base.
# $(1): the base dir, relative to the root of the source tree.
# $(2): the file name pattern to be passed to find as "-name".
# $(3): a list of subdirs of the base dir.
# Returns: a list of paths relative to the base dir.
###########################################################
define find-files-in-subdirs
$(sort $(patsubst ./%,%, \
  $(shell cd $(1) ; \
          find -L $(3) -name $(2) -and -not -name ".*") \
 ))
endef

###########################################################
## Find all of the files under the named directories with
## the specified name.
## Meant to be used like:
##    SRC_FILES := $(call all-named-files-under,*.h,src tests)
###########################################################
define all-named-files-under
$(call find-files-in-subdirs,$(LOCAL_PATH),"$(1)",$(2))
endef

###########################################################
## Look in the named list of directories for makefiles,
## relative to the current directory.
## Must be called before including any other makefile!!
###########################################################
# $(1): List of directories to look for under this directory
define all-named-subdir-makefiles
$(wildcard $(addsuffix /Android.mk, $(addprefix $(call my-dir)/,$(1))))
endef

###########################################################
## Retrieve a list of all makefiles immediately below some directory
###########################################################
define all-module-makefiles-under
$(wildcard $(1)/*/module.mk)
endef

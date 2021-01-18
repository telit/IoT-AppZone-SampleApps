CPPFLAGS=
LDFLAGS=
CFLAGS=
OBJS=
include prod_tool.in
-include Makefile.in
#https://www.gnu.org/software/make/manual/html_node/Include.html#Include
include export.mk

#build using packaged libs
ifeq ($(PRODUCT), 2G)
	PRODUCT = TELIT_2G_I
	AZ_MODEL = GE910
endif

ifeq ($(PRODUCT), 3G)
	PRODUCT = TELIT_3G_I
	AZ_MODEL = HE910
endif

ifeq ($(PRODUCT), 4G)
	PRODUCT = TELIT_4G_I
	AZ_MODEL = LE910v2
endif

ifeq ($(PRODUCT), 4GM)
	PRODUCT = TELIT_4G_I
	AZ_MODEL = ME910
endif

ifeq ($(PRODUCT), 4GMLEGACY)
	PRODUCT = TELIT_4G_I
	AZ_MODEL = ME910_LEGACY
endif


space :=
space +=
makefilePath := $(APPZONE_MAKEFILE_COMMON)/$(AZ_BASE_MAKEFILE)
makefilePath := $(subst $(space),\$(space),$(makefilePath))
include $(makefilePath)

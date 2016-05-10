# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: import/tools/imageProcs/sgpe_image.mk $
#
# OpenPOWER HCODE Project
#
# COPYRIGHT 2016,2017
# [+] International Business Machines Corp.
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.
#
# IBM_PROLOG_END_TAG
IMAGE=sgpe_image

# add dependency on the raw image.bin file
SGPE_DEPS=$$($(IMAGE)_PATH)/.$(IMAGE).setbuild_user

# dependencies for bin files needed in the sgpe xip image
QPMR_BIN_FILE=$(IMAGEPATH)/qpmr_header/qpmr_header.bin
SGPE_BIN_FILE=$(IMAGEPATH)/stop_gpe/stop_gpe.bin

$(call XIP_TOOL,append,.qpmr,$(SGPE_DEPS) $(QPMR_BIN_FILE) ,$(QPMR_BIN_FILE))
$(call APPEND_EMPTY_SECTION,lvl1_bl,1024,$$($(IMAGE)_PATH)/.$(IMAGE).append.qpmr)
$(call APPEND_EMPTY_SECTION,lvl2_bl,1024,$$($(IMAGE)_PATH)/.$(IMAGE).append.lvl1_bl)
$(call XIP_TOOL,append,.hcode, $$($(IMAGE)_PATH)/.$(IMAGE).append.lvl2_bl $(SGPE_BIN_FILE),$(SGPE_BIN_FILE))
$(call XIP_TOOL,report,,$$($(IMAGE)_PATH)/.$(IMAGE).append.hcode)
$(call BUILD_XIPIMAGE)

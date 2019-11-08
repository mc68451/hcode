# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: import/chips/p10/procedures/ppe/hwp/p10_qme_putringutils.mk $
#
# OpenPOWER EKB Project
#
# COPYRIGHT 2019
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
ifeq ($(SCAN_TEST),1)
PROCEDURE=p10_qme_putringutils
$(call ADD_MODULE_INCDIR,$(PROCEDURE),$(ROOTPATH)/chips/p10/procedures/hwp/lib/)
$(call ADD_MODULE_INCDIR,$(PROCEDURE),$(ROOTPATH)/chips/p10/utils/imageProcs/)
$(call ADD_MODULE_INCDIR,$(PROCEDURE),$(ROOTPATH)/chips/p10/common/ppe/powmanlib/)
$(call ADD_MODULE_INCDIR,$(PROCEDURE),$(ROOTPATH)/chips/p10/procedures/ppe/pk/ppe42/)
$(call ADD_MODULE_INCDIR,$(PROCEDURE),$(ROOTPATH)/chips/p10/common/ppe/baselib/)
$(call ADD_MODULE_INCDIR,$(PROCEDURE),$(ROOTPATH)/chips/p10/common/include/)
$(call ADD_MODULE_INCDIR,$(PROCEDURE),$(ROOTPATH)/chips/p10/common/ppe/iota/)
$(call ADD_MODULE_INCDIR,$(PROCEDURE),$(ROOTPATH)/chips/p10/procedures/ppe/qme/)
$(call BUILD_PROCEDURE)
endif

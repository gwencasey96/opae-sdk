// Copyright(c) 2018, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include "opae_int.h"
#include "pluginmgr.h"

#include "safe_string/safe_string.h"

fpga_result fpgaInitialize(const char *config_file)
{
	return opae_plugin_mgr_initialize(config_file) ? FPGA_EXCEPTION : FPGA_OK;
}

fpga_result fpgaOpen(fpga_token token, fpga_handle *handle, int flags)
{
	fpga_result res;
	opae_wrapped_token *wrapped_token;
	fpga_handle opae_handle = NULL;
	opae_wrapped_handle *wrapped_handle;

	wrapped_token = opae_validate_wrapped_token(token);

	ASSERT_NOT_NULL(wrapped_token);
	ASSERT_NOT_NULL(handle);
	ASSERT_NOT_NULL_RESULT(wrapped_token->adapter_table->fpgaOpen,
			       FPGA_NOT_SUPPORTED);

	res = wrapped_token->adapter_table->fpgaOpen(wrapped_token->opae_token,
						     &opae_handle,
						     flags);

	ASSERT_RESULT(res);

	wrapped_handle = opae_allocate_wrapped_handle(wrapped_token,
						      opae_handle,
						      wrapped_token->adapter_table);

	ASSERT_NOT_NULL_RESULT(wrapped_handle, FPGA_NO_MEMORY);

	*handle = wrapped_handle;

	return res;
}

fpga_result fpgaClose(fpga_handle handle)
{
	fpga_result res;
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(handle);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL_RESULT(wrapped_handle->adapter_table->fpgaClose,
			       FPGA_NOT_SUPPORTED);

	res = wrapped_handle->adapter_table->fpgaClose(wrapped_handle->opae_handle);

	opae_destroy_wrapped_handle(wrapped_handle);

	return res;
}

fpga_result fpgaReset(fpga_handle handle)
{
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(handle);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL_RESULT(wrapped_handle->adapter_table->fpgaReset,
			       FPGA_NOT_SUPPORTED);

	return wrapped_handle->adapter_table->fpgaReset(wrapped_handle->opae_handle);
}

fpga_result fpgaGetPropertiesFromHandle(fpga_handle handle,
					fpga_properties *prop)
{
	fpga_result res;
	fpga_result dres = FPGA_OK;
	fpga_properties pr = NULL;
	opae_wrapped_properties *wrapped_properties;
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(handle);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL(prop);
	ASSERT_NOT_NULL_RESULT(
		wrapped_handle->adapter_table->fpgaGetPropertiesFromHandle,
		FPGA_NOT_SUPPORTED);
	ASSERT_NOT_NULL_RESULT(
		wrapped_handle->adapter_table->fpgaDestroyProperties,
		FPGA_NOT_SUPPORTED);

	res = wrapped_handle->adapter_table->fpgaGetPropertiesFromHandle(
					wrapped_handle->opae_handle, &pr);

	ASSERT_RESULT(res);
	
	wrapped_properties = opae_allocate_wrapped_properties(pr,
						wrapped_handle->adapter_table);

	if (!wrapped_properties) {
		OPAE_ERR("malloc failed");
		res = FPGA_NO_MEMORY;
		dres = wrapped_handle->adapter_table->fpgaDestroyProperties(&pr);
	}

	*prop = wrapped_properties;

	return res != FPGA_OK ? res : dres;
}

fpga_result fpgaGetProperties(fpga_token token, fpga_properties *prop)
{
	fpga_result res;
	fpga_result dres = FPGA_OK;
	fpga_properties pr = NULL;
	opae_wrapped_properties *wrapped_properties;
	opae_wrapped_token *wrapped_token = opae_validate_wrapped_token(token);

	ASSERT_NOT_NULL(wrapped_token);
	ASSERT_NOT_NULL(prop);
	ASSERT_NOT_NULL_RESULT(
		wrapped_token->adapter_table->fpgaGetProperties,
		FPGA_NOT_SUPPORTED);
	ASSERT_NOT_NULL_RESULT(
		wrapped_token->adapter_table->fpgaDestroyProperties,
		FPGA_NOT_SUPPORTED);

	res = wrapped_token->adapter_table->fpgaGetProperties(
					wrapped_token->opae_token, &pr);

	ASSERT_RESULT(res);

	wrapped_properties = opae_allocate_wrapped_properties(pr,
						wrapped_token->adapter_table);

	if (!wrapped_properties) {
		OPAE_ERR("malloc failed");
		res = FPGA_NO_MEMORY;
		dres = wrapped_token->adapter_table->fpgaDestroyProperties(&pr);
	}

	*prop = wrapped_properties;

	return res != FPGA_OK ? res : dres;
}

fpga_result fpgaUpdateProperties(fpga_token token, fpga_properties prop)
{
	opae_wrapped_token *wrapped_token = opae_validate_wrapped_token(token);
	opae_wrapped_properties *wrapped_properties =
		opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_token);
	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(wrapped_token->adapter_table->fpgaUpdateProperties,
			       FPGA_NOT_SUPPORTED);

	return wrapped_token->adapter_table->fpgaUpdateProperties(
			wrapped_token->opae_token,
			wrapped_properties->opae_properties);
}

fpga_result fpgaClearProperties(fpga_properties prop)
{
	opae_wrapped_properties *wrapped_properties =
		opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(wrapped_properties->adapter_table->fpgaClearProperties,
			       FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaClearProperties(
			wrapped_properties->opae_properties);
}

fpga_result fpgaCloneProperties(fpga_properties src, fpga_properties *dst)
{
	fpga_result res;
	fpga_result dres = FPGA_OK;
	fpga_properties cloned_properties = NULL;
	opae_wrapped_properties *wrapped_dst_properties;
	opae_wrapped_properties *wrapped_src_properties =
			opae_validate_wrapped_properties(src);

	ASSERT_NOT_NULL(wrapped_src_properties);
	ASSERT_NOT_NULL(dst);
	ASSERT_NOT_NULL_RESULT(
		wrapped_src_properties->adapter_table->fpgaCloneProperties,
		FPGA_NOT_SUPPORTED);
	ASSERT_NOT_NULL_RESULT(
		wrapped_src_properties->adapter_table->fpgaDestroyProperties,
		FPGA_NOT_SUPPORTED);

	res = wrapped_src_properties->adapter_table->fpgaCloneProperties(
			wrapped_src_properties->opae_properties,
			&cloned_properties);

	ASSERT_RESULT(res);

	wrapped_dst_properties = opae_allocate_wrapped_properties(
			cloned_properties,
			wrapped_src_properties->adapter_table);

	if (!wrapped_dst_properties) {
		OPAE_ERR("malloc failed");
		res = FPGA_NO_MEMORY;
		dres = wrapped_src_properties->adapter_table->fpgaDestroyProperties(
				&cloned_properties);
	}

	*dst = wrapped_dst_properties;

	return res != FPGA_OK ? res : dres;
}

fpga_result fpgaDestroyProperties(fpga_properties *prop)
{
	fpga_result res;
	opae_wrapped_properties *wrapped_properties;

	ASSERT_NOT_NULL(prop);

	wrapped_properties = opae_validate_wrapped_properties(*prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(wrapped_properties->adapter_table->fpgaDestroyProperties,
			       FPGA_NOT_SUPPORTED);

	res = wrapped_properties->adapter_table->fpgaDestroyProperties(
			&wrapped_properties->opae_properties);

	opae_destroy_wrapped_properties(wrapped_properties);

	return res;
}

fpga_result fpgaPropertiesGetParent(const fpga_properties prop,
				    fpga_token *parent)
{
	fpga_result res;
	fpga_token tok = NULL;
	opae_wrapped_token *wrapped_token;
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(parent);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetParent,
		FPGA_NOT_SUPPORTED);

	res = wrapped_properties->adapter_table->fpgaPropertiesGetParent(
			wrapped_properties->opae_properties,
			&tok);

	ASSERT_RESULT(res);

	wrapped_token = opae_allocate_wrapped_token(tok,
				wrapped_properties->adapter_table);

	if (!wrapped_token) {
		OPAE_ERR("malloc failed");
		res = FPGA_NO_MEMORY;
	}

	*parent = wrapped_token;

	return res;
}

fpga_result fpgaPropertiesSetParent(fpga_properties prop, fpga_token parent)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);
	opae_wrapped_token *wrapped_token =
			opae_validate_wrapped_token(parent);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(wrapped_token);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetParent,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetParent(
			wrapped_properties->opae_properties,
			wrapped_token->opae_token);
}

fpga_result fpgaPropertiesGetObjectType(const fpga_properties prop,
					fpga_objtype *objtype)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(objtype);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetObjectType,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetObjectType(
			wrapped_properties->opae_properties,
			objtype);
}

fpga_result fpgaPropertiesSetObjectType(fpga_properties prop,
					fpga_objtype objtype)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetObjectType,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetObjectType(
			wrapped_properties->opae_properties,
			objtype);
}

fpga_result fpgaPropertiesGetSegment(const fpga_properties prop,
				     uint16_t *segment)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(segment);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetSegment,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetSegment(
			wrapped_properties->opae_properties,
			segment);
}

fpga_result fpgaPropertiesSetSegment(fpga_properties prop, uint16_t segment)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetSegment,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetSegment(
			wrapped_properties->opae_properties,
			segment);
}

fpga_result fpgaPropertiesGetBus(const fpga_properties prop, uint8_t *bus)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(bus);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetBus,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetBus(
			wrapped_properties->opae_properties,
			bus);
}

fpga_result fpgaPropertiesSetBus(fpga_properties prop, uint8_t bus)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetBus,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetBus(
			wrapped_properties->opae_properties,
			bus);
}

fpga_result fpgaPropertiesGetDevice(const fpga_properties prop, uint8_t *device)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(device);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetDevice,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetDevice(
			wrapped_properties->opae_properties,
			device);
}

fpga_result fpgaPropertiesSetDevice(fpga_properties prop, uint8_t device)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetDevice,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetDevice(
			wrapped_properties->opae_properties,
			device);
}

fpga_result fpgaPropertiesGetFunction(const fpga_properties prop,
				      uint8_t *function)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(function);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetFunction,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetFunction(
			wrapped_properties->opae_properties,
			function);
}

fpga_result fpgaPropertiesSetFunction(fpga_properties prop, uint8_t function)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetFunction,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetFunction(
			wrapped_properties->opae_properties,
			function);
}

fpga_result fpgaPropertiesGetSocketID(const fpga_properties prop,
				      uint8_t *socket_id)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(socket_id);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetSocketID,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetSocketID(
			wrapped_properties->opae_properties,
			socket_id);
}

fpga_result fpgaPropertiesSetSocketID(fpga_properties prop, uint8_t socket_id)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetSocketID,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetSocketID(
			wrapped_properties->opae_properties,
			socket_id);
}

fpga_result fpgaPropertiesGetDeviceID(const fpga_properties prop,
				      uint16_t *device_id)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(device_id);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetDeviceID,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetDeviceID(
			wrapped_properties->opae_properties,
			device_id);
}

fpga_result fpgaPropertiesSetDeviceID(fpga_properties prop, uint16_t device_id)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetDeviceID,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetDeviceID(
			wrapped_properties->opae_properties,
			device_id);
}

fpga_result fpgaPropertiesGetNumSlots(const fpga_properties prop,
				      uint32_t *num_slots)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(num_slots);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetNumSlots,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetNumSlots(
			wrapped_properties->opae_properties,
			num_slots);
}

fpga_result fpgaPropertiesSetNumSlots(fpga_properties prop, uint32_t num_slots)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetNumSlots,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetNumSlots(
			wrapped_properties->opae_properties,
			num_slots);
}

fpga_result fpgaPropertiesGetBBSID(const fpga_properties prop, uint64_t *bbs_id)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(bbs_id);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetBBSID,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetBBSID(
			wrapped_properties->opae_properties,
			bbs_id);
}

fpga_result fpgaPropertiesSetBBSID(fpga_properties prop, uint64_t bbs_id)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetBBSID,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetBBSID(
			wrapped_properties->opae_properties,
			bbs_id);
}

fpga_result fpgaPropertiesGetBBSVersion(const fpga_properties prop,
					fpga_version *bbs_version)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(bbs_version);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetBBSVersion,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetBBSVersion(
			wrapped_properties->opae_properties,
			bbs_version);
}

fpga_result fpgaPropertiesSetBBSVersion(fpga_properties prop,
					fpga_version version)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetBBSVersion,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetBBSVersion(
			wrapped_properties->opae_properties,
			version);
}

fpga_result fpgaPropertiesGetVendorID(const fpga_properties prop,
				      uint16_t *vendor_id)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(vendor_id);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetVendorID,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetVendorID(
			wrapped_properties->opae_properties,
			vendor_id);
}

fpga_result fpgaPropertiesSetVendorID(fpga_properties prop, uint16_t vendor_id)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetVendorID,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetVendorID(
			wrapped_properties->opae_properties,
			vendor_id);
}

fpga_result fpgaPropertiesGetModel(const fpga_properties prop, char *model)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(model);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetModel,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetModel(
			wrapped_properties->opae_properties,
			model);
}

fpga_result fpgaPropertiesSetModel(fpga_properties prop, char *model)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(model);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetModel,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetModel(
			wrapped_properties->opae_properties,
			model);
}

fpga_result fpgaPropertiesGetLocalMemorySize(const fpga_properties prop,
					     uint64_t *lms)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(lms);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetLocalMemorySize,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetLocalMemorySize(
			wrapped_properties->opae_properties,
			lms);
}

fpga_result fpgaPropertiesSetLocalMemorySize(fpga_properties prop, uint64_t lms)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetLocalMemorySize,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetLocalMemorySize(
			wrapped_properties->opae_properties,
			lms);
}

fpga_result fpgaPropertiesGetCapabilities(const fpga_properties prop,
					  uint64_t *capabilities)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(capabilities);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetCapabilities,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetCapabilities(
			wrapped_properties->opae_properties,
			capabilities);
}

fpga_result fpgaPropertiesSetCapabilities(fpga_properties prop,
					  uint64_t capabilities)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetCapabilities,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetCapabilities(
			wrapped_properties->opae_properties,
			capabilities);
}

fpga_result fpgaPropertiesGetGUID(const fpga_properties prop, fpga_guid *guid)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(guid);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetGUID,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetGUID(
			wrapped_properties->opae_properties,
			guid);
}

fpga_result fpgaPropertiesSetGUID(fpga_properties prop, fpga_guid guid)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetGUID,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetGUID(
			wrapped_properties->opae_properties,
			guid);
}

fpga_result fpgaPropertiesGetNumMMIO(const fpga_properties prop,
				     uint32_t *mmio_spaces)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(mmio_spaces);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetNumMMIO,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetNumMMIO(
			wrapped_properties->opae_properties,
			mmio_spaces);
}

fpga_result fpgaPropertiesSetNumMMIO(fpga_properties prop, uint32_t mmio_spaces)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetNumMMIO,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetNumMMIO(
			wrapped_properties->opae_properties,
			mmio_spaces);
}

fpga_result fpgaPropertiesGetNumInterrupts(const fpga_properties prop,
					   uint32_t *num_interrupts)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(num_interrupts);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetNumInterrupts,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetNumInterrupts(
			wrapped_properties->opae_properties,
			num_interrupts);
}

fpga_result fpgaPropertiesSetNumInterrupts(fpga_properties prop,
					   uint32_t num_interrupts)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetNumInterrupts,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetNumInterrupts(
			wrapped_properties->opae_properties,
			num_interrupts);
}

fpga_result fpgaPropertiesGetAcceleratorState(const fpga_properties prop,
					      fpga_accelerator_state *state)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(state);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetAcceleratorState,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetAcceleratorState(
			wrapped_properties->opae_properties,
			state);
}

fpga_result fpgaPropertiesSetAcceleratorState(fpga_properties prop,
					      fpga_accelerator_state state)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetAcceleratorState,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetAcceleratorState(
			wrapped_properties->opae_properties,
			state);
}

fpga_result fpgaPropertiesGetObjectID(const fpga_properties prop,
				      uint64_t *object_id)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(object_id);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetObjectID,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetObjectID(
			wrapped_properties->opae_properties,
			object_id);
}

fpga_result fpgaPropertiesSetObjectID(const fpga_properties prop,
				      uint64_t object_id)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetObjectID,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetObjectID(
			wrapped_properties->opae_properties,
			object_id);
}

fpga_result fpgaPropertiesGetNumErrors(const fpga_properties prop,
				       uint32_t *num_errors)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL(num_errors);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesGetNumErrors,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesGetNumErrors(
			wrapped_properties->opae_properties,
			num_errors);
}

fpga_result fpgaPropertiesSetNumErrors(const fpga_properties prop,
				       uint32_t num_errors)
{
	opae_wrapped_properties *wrapped_properties =
			opae_validate_wrapped_properties(prop);

	ASSERT_NOT_NULL(wrapped_properties);
	ASSERT_NOT_NULL_RESULT(
		wrapped_properties->adapter_table->fpgaPropertiesSetNumErrors,
		FPGA_NOT_SUPPORTED);

	return wrapped_properties->adapter_table->fpgaPropertiesSetNumErrors(
			wrapped_properties->opae_properties,
			num_errors);
}

fpga_result fpgaWriteMMIO64(fpga_handle handle, uint32_t mmio_num,
			    uint64_t offset, uint64_t value)
{
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(handle);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL_RESULT(wrapped_handle->adapter_table->fpgaWriteMMIO64,
			       FPGA_NOT_SUPPORTED);

	return wrapped_handle->adapter_table->fpgaWriteMMIO64(
			wrapped_handle->opae_handle, mmio_num,
			offset, value);
}

fpga_result fpgaReadMMIO64(fpga_handle handle, uint32_t mmio_num,
			   uint64_t offset, uint64_t *value)
{
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(handle);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL_RESULT(wrapped_handle->adapter_table->fpgaReadMMIO64,
			       FPGA_NOT_SUPPORTED);

	return wrapped_handle->adapter_table->fpgaReadMMIO64(
			wrapped_handle->opae_handle, mmio_num,
			offset, value);
}

fpga_result fpgaWriteMMIO32(fpga_handle handle, uint32_t mmio_num,
			    uint64_t offset, uint32_t value)
{
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(handle);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL_RESULT(wrapped_handle->adapter_table->fpgaWriteMMIO32,
			       FPGA_NOT_SUPPORTED);

	return wrapped_handle->adapter_table->fpgaWriteMMIO32(
			wrapped_handle->opae_handle, mmio_num,
			offset, value);
}

fpga_result fpgaReadMMIO32(fpga_handle handle, uint32_t mmio_num,
			   uint64_t offset, uint32_t *value)
{
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(handle);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL_RESULT(wrapped_handle->adapter_table->fpgaReadMMIO32,
			       FPGA_NOT_SUPPORTED);

	return wrapped_handle->adapter_table->fpgaReadMMIO32(
			wrapped_handle->opae_handle, mmio_num,
			offset, value);
}

fpga_result fpgaMapMMIO(fpga_handle handle, uint32_t mmio_num,
			uint64_t **mmio_ptr)
{
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(handle);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL_RESULT(wrapped_handle->adapter_table->fpgaMapMMIO,
			       FPGA_NOT_SUPPORTED);

	return wrapped_handle->adapter_table->fpgaMapMMIO(
			wrapped_handle->opae_handle, mmio_num, mmio_ptr);
}

fpga_result fpgaUnmapMMIO(fpga_handle handle, uint32_t mmio_num)
{
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(handle);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL_RESULT(wrapped_handle->adapter_table->fpgaUnmapMMIO,
			       FPGA_NOT_SUPPORTED);

	return wrapped_handle->adapter_table->fpgaUnmapMMIO(
			wrapped_handle->opae_handle, mmio_num);
}

typedef struct _opae_enumeration_context {
	// <verbatim from fpgaEnumerate>
	const fpga_properties *filters;
	uint32_t num_filters;
	fpga_token *wrapped_tokens;
	uint32_t max_wrapped_tokens;
	uint32_t *num_matches;
	// </verbatim from fpgaEnumerate>

	fpga_token *adapter_tokens;
	uint32_t num_wrapped_tokens;
	uint32_t errors;
} opae_enumeration_context;

static int opae_enumerate(const opae_api_adapter_table *adapter, void *context)
{
	opae_enumeration_context *ctx = (opae_enumeration_context *) context;
	fpga_result res;
	uint32_t num_matches = 0;
	uint32_t i;
	uint32_t space_remaining;

	// TODO: accept/reject this adapter, based on device support
	if (adapter->supports_device) {

	}

	// TODO: accept/reject this adapter, based on host support
	if (adapter->supports_host) {

	}

	space_remaining = ctx->max_wrapped_tokens - ctx->num_wrapped_tokens;

	if (ctx->wrapped_tokens && !space_remaining)
		return OPAE_ENUM_STOP;

	if (!adapter->fpgaEnumerate) {
		OPAE_MSG("NULL fpgaEnumerate in adapter \"%s\"",
				adapter->plugin.path);
		return OPAE_ENUM_CONTINUE;
	}

	res = adapter->fpgaEnumerate(ctx->filters,
				     ctx->num_filters,
				     ctx->adapter_tokens,
				     space_remaining,
				     &num_matches);

	if (res != FPGA_OK) {
		OPAE_ERR("fpgaEnumerate() failed for \"%s\"",
				adapter->plugin.path);
		++ctx->errors;
		return OPAE_ENUM_CONTINUE;
	}

	*ctx->num_matches += num_matches;

	if (!ctx->adapter_tokens) {
		// requesting token count, only.
		return OPAE_ENUM_CONTINUE;
	}

	for (i = 0 ; i < space_remaining ; ++i) {
		opae_wrapped_token *wt =
			opae_allocate_wrapped_token(ctx->adapter_tokens[i],
						    adapter);
		if (!wt) {
			++ctx->errors;
			return OPAE_ENUM_STOP;
		}

		ctx->wrapped_tokens[ctx->num_wrapped_tokens++] = wt;
	}

	return ctx->num_wrapped_tokens == ctx->max_wrapped_tokens ?
		OPAE_ENUM_STOP : OPAE_ENUM_CONTINUE;
}

fpga_result fpgaEnumerate(const fpga_properties *filters, uint32_t num_filters,
			  fpga_token *tokens, uint32_t max_tokens,
			  uint32_t *num_matches)
{
	fpga_result res = FPGA_EXCEPTION;
	fpga_token *adapter_tokens = NULL;

	opae_enumeration_context enum_context;


	ASSERT_NOT_NULL(num_matches);

	if ((max_tokens > 0) && !tokens) {
		OPAE_ERR("max_tokens > 0 with NULL tokens");
		return FPGA_INVALID_PARAM;
	}

	if ((num_filters > 0) && !filters) {
		OPAE_ERR("num_filters > 0 with NULL filters");
		return FPGA_INVALID_PARAM;
	}

	if ((num_filters == 0) && (filters != NULL)) {
		OPAE_ERR("num_filters == 0 with non-NULL filters");
		return FPGA_INVALID_PARAM;
	}

	*num_matches = 0;

	enum_context.filters = filters;
	enum_context.num_filters = num_filters;
	enum_context.wrapped_tokens = tokens;
	enum_context.max_wrapped_tokens = max_tokens;
	enum_context.num_matches = num_matches;

	if (tokens) {
		adapter_tokens = (fpga_token *) calloc(max_tokens, sizeof(fpga_token));
		if (!adapter_tokens) {
			OPAE_ERR("out of memory");
			return FPGA_NO_MEMORY;
		}
	}

	enum_context.adapter_tokens = adapter_tokens;
	enum_context.num_wrapped_tokens = 0;
	enum_context.errors = 0;

	opae_plugin_mgr_for_each_adapter(opae_enumerate, &enum_context);

	res = (enum_context.errors > 0) ? FPGA_EXCEPTION : FPGA_OK;

	if (adapter_tokens)
		free(adapter_tokens);

	return res;
}

fpga_result fpgaCloneToken(fpga_token src, fpga_token *dst)
{
	fpga_result res;
	fpga_result dres = FPGA_OK;
	fpga_token cloned_token = NULL;
	opae_wrapped_token *wrapped_dst_token;
	opae_wrapped_token *wrapped_src_token = opae_validate_wrapped_token(src);

	ASSERT_NOT_NULL(wrapped_src_token);
	ASSERT_NOT_NULL(dst);
	ASSERT_NOT_NULL_RESULT(wrapped_src_token->adapter_table->fpgaCloneToken,
			       FPGA_NOT_SUPPORTED);
	ASSERT_NOT_NULL_RESULT(wrapped_src_token->adapter_table->fpgaDestroyToken,
			       FPGA_NOT_SUPPORTED);

	res = wrapped_src_token->adapter_table->fpgaCloneToken(
			wrapped_src_token->opae_token, &cloned_token);

	ASSERT_RESULT(res);

	wrapped_dst_token = opae_allocate_wrapped_token(cloned_token,
				wrapped_src_token->adapter_table);

	if (!wrapped_dst_token) {
		OPAE_ERR("malloc failed");
		res = FPGA_NO_MEMORY;
		dres = wrapped_src_token->adapter_table->fpgaDestroyToken(
				&cloned_token);
	}

	*dst = wrapped_dst_token;

	return res != FPGA_OK ? res : dres;
}

fpga_result fpgaDestroyToken(fpga_token *token)
{
	fpga_result res;
	opae_wrapped_token *wrapped_token;

	ASSERT_NOT_NULL(token);

	wrapped_token = opae_validate_wrapped_token(*token);

	ASSERT_NOT_NULL(wrapped_token);
	ASSERT_NOT_NULL_RESULT(wrapped_token->adapter_table->fpgaDestroyToken,
			       FPGA_NOT_SUPPORTED);

	res = wrapped_token->adapter_table->fpgaDestroyToken(
			&wrapped_token->opae_token);

	opae_destroy_wrapped_token(wrapped_token);

	return res;
}

fpga_result fpgaGetNumUmsg(fpga_handle handle, uint64_t *value)
{
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(handle);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL(value);
	ASSERT_NOT_NULL_RESULT(wrapped_handle->adapter_table->fpgaGetNumUmsg,
			       FPGA_NOT_SUPPORTED);

	return wrapped_handle->adapter_table->fpgaGetNumUmsg(
			wrapped_handle->opae_handle, value);
}

fpga_result fpgaSetUmsgAttributes(fpga_handle handle, uint64_t value)
{
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(handle);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL_RESULT(wrapped_handle->adapter_table->fpgaSetUmsgAttributes,
			       FPGA_NOT_SUPPORTED);

	return wrapped_handle->adapter_table->fpgaSetUmsgAttributes(
			wrapped_handle->opae_handle, value);
}

fpga_result fpgaTriggerUmsg(fpga_handle handle, uint64_t value)
{
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(handle);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL_RESULT(wrapped_handle->adapter_table->fpgaTriggerUmsg,
			       FPGA_NOT_SUPPORTED);

	return wrapped_handle->adapter_table->fpgaTriggerUmsg(
			wrapped_handle->opae_handle, value);
}

fpga_result fpgaGetUmsgPtr(fpga_handle handle, uint64_t **umsg_ptr)
{
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(handle);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL(umsg_ptr);
	ASSERT_NOT_NULL_RESULT(wrapped_handle->adapter_table->fpgaGetUmsgPtr,
			       FPGA_NOT_SUPPORTED);

	return wrapped_handle->adapter_table->fpgaGetUmsgPtr(
			wrapped_handle->opae_handle, umsg_ptr);
}

fpga_result fpgaPrepareBuffer(fpga_handle handle, uint64_t len, void **buf_addr,
			      uint64_t *wsid, int flags)
{
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(handle);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL(buf_addr);
	ASSERT_NOT_NULL(wsid);
	ASSERT_NOT_NULL_RESULT(wrapped_handle->adapter_table->fpgaPrepareBuffer,
			       FPGA_NOT_SUPPORTED);

	return wrapped_handle->adapter_table->fpgaPrepareBuffer(
			wrapped_handle->opae_handle, len, buf_addr,
			wsid, flags);
}

fpga_result fpgaReleaseBuffer(fpga_handle handle, uint64_t wsid)
{
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(handle);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL_RESULT(wrapped_handle->adapter_table->fpgaReleaseBuffer,
			       FPGA_NOT_SUPPORTED);

	return wrapped_handle->adapter_table->fpgaReleaseBuffer(
			wrapped_handle->opae_handle, wsid);
}

fpga_result fpgaGetIOAddress(fpga_handle handle, uint64_t wsid,
			     uint64_t *ioaddr)
{
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(handle);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL(ioaddr);
	ASSERT_NOT_NULL_RESULT(wrapped_handle->adapter_table->fpgaGetIOAddress,
			       FPGA_NOT_SUPPORTED);

	return wrapped_handle->adapter_table->fpgaGetIOAddress(
			wrapped_handle->opae_handle, wsid, ioaddr);
}

fpga_result fpgaGetOPAECVersion(fpga_version *version)
{
	ASSERT_NOT_NULL(version);

        version->major = INTEL_FPGA_API_VER_MAJOR;
        version->minor = INTEL_FPGA_API_VER_MINOR;
        version->patch = INTEL_FPGA_API_VER_REV;

        return FPGA_OK;
}

fpga_result fpgaGetOPAECVersionString(char *version_str, size_t len)
{
        errno_t err;

	ASSERT_NOT_NULL(version_str);

        err = strncpy_s(version_str, len, INTEL_FPGA_API_VERSION,
                  sizeof(INTEL_FPGA_API_VERSION));

        if (err) {
                OPAE_ERR("strncpy_s failed with error %d", err);
                return FPGA_EXCEPTION;
        }

        return FPGA_OK;
}

fpga_result fpgaGetOPAECBuildString(char *build_str, size_t len)
{
        errno_t err;

	ASSERT_NOT_NULL(build_str);

        err = strncpy_s(build_str, len, INTEL_FPGA_API_HASH,
                  sizeof(INTEL_FPGA_API_HASH));

        if (err) {
                OPAE_ERR("strncpy_s failed with error %d", err);
                return FPGA_EXCEPTION;
        }

        return FPGA_OK;
}

fpga_result fpgaReadError(fpga_token token, uint32_t error_num, uint64_t *value)
{
	opae_wrapped_token *wrapped_token = opae_validate_wrapped_token(token);

	ASSERT_NOT_NULL(wrapped_token);
	ASSERT_NOT_NULL(value);
	ASSERT_NOT_NULL_RESULT(wrapped_token->adapter_table->fpgaReadError,
			       FPGA_NOT_SUPPORTED);

	return wrapped_token->adapter_table->fpgaReadError(
			wrapped_token->opae_token, error_num, value);
}

fpga_result fpgaClearError(fpga_token token, uint32_t error_num)
{
	opae_wrapped_token *wrapped_token = opae_validate_wrapped_token(token);

	ASSERT_NOT_NULL(wrapped_token);
	ASSERT_NOT_NULL_RESULT(wrapped_token->adapter_table->fpgaClearError,
			       FPGA_NOT_SUPPORTED);

	return wrapped_token->adapter_table->fpgaClearError(
			wrapped_token->opae_token, error_num);
}

fpga_result fpgaClearAllErrors(fpga_token token)
{
	opae_wrapped_token *wrapped_token = opae_validate_wrapped_token(token);

	ASSERT_NOT_NULL(wrapped_token);
	ASSERT_NOT_NULL_RESULT(wrapped_token->adapter_table->fpgaClearAllErrors,
			       FPGA_NOT_SUPPORTED);

	return wrapped_token->adapter_table->fpgaClearAllErrors(
			wrapped_token->opae_token);
}

fpga_result fpgaGetErrorInfo(fpga_token token, uint32_t error_num,
			     struct fpga_error_info *error_info)
{
	opae_wrapped_token *wrapped_token = opae_validate_wrapped_token(token);

	ASSERT_NOT_NULL(wrapped_token);
	ASSERT_NOT_NULL(error_info);
	ASSERT_NOT_NULL_RESULT(wrapped_token->adapter_table->fpgaGetErrorInfo,
			       FPGA_NOT_SUPPORTED);

	return wrapped_token->adapter_table->fpgaGetErrorInfo(
			wrapped_token->opae_token, error_num, error_info);
}

const char *fpgaErrStr(fpga_result e)
{
	switch (e) {
	case FPGA_OK:
		return "success";
	case FPGA_INVALID_PARAM:
		return "invalid parameter";
	case FPGA_BUSY:
		return "resource busy";
	case FPGA_EXCEPTION:
		return "exception";
	case FPGA_NOT_FOUND:
		return "not found";
	case FPGA_NO_MEMORY:
		return "no memory";
	case FPGA_NOT_SUPPORTED:
		return "not supported";
	case FPGA_NO_DRIVER:
		return "no driver available";
	case FPGA_NO_DAEMON:
		return "no fpga daemon running";
	case FPGA_NO_ACCESS:
		return "insufficient privileges";
	case FPGA_RECONF_ERROR:
		return "reconfiguration error";
	default:
		return "unknown error";
	}
}

fpga_result fpgaCreateEventHandle(fpga_event_handle *event_handle)
{
//	opae_wrapped_event_handle *wrapped_event_handle;
//	fpga_event_handle event = NULL;

	ASSERT_NOT_NULL(event_handle);




	return FPGA_NOT_SUPPORTED;
}

fpga_result fpgaDestroyEventHandle(fpga_event_handle *event_handle)
{
	ASSERT_NOT_NULL(event_handle);

	return FPGA_NOT_SUPPORTED;
}

fpga_result fpgaGetOSObjectFromEventHandle(const fpga_event_handle eh, int *fd)
{

	return FPGA_NOT_SUPPORTED;
}

fpga_result fpgaRegisterEvent(fpga_handle handle, fpga_event_type event_type,
			      fpga_event_handle event_handle, uint32_t flags)
{

	return FPGA_NOT_SUPPORTED;
}

fpga_result fpgaUnregisterEvent(fpga_handle handle, fpga_event_type event_type,
				fpga_event_handle event_handle)
{



	return FPGA_NOT_SUPPORTED;
}

fpga_result fpgaAssignPortToInterface(fpga_handle fpga, uint32_t interface_num,
				      uint32_t slot_num, int flags)
{
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(fpga);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL_RESULT(wrapped_handle->adapter_table->fpgaAssignPortToInterface,
			       FPGA_NOT_SUPPORTED);

	return wrapped_handle->adapter_table->fpgaAssignPortToInterface(
			wrapped_handle->opae_handle, interface_num,
			slot_num, flags);
}

fpga_result fpgaAssignToInterface(fpga_handle fpga, fpga_token accelerator,
				  uint32_t host_interface, int flags)
{
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(fpga);
	opae_wrapped_token  *wrapped_token  = opae_validate_wrapped_token(accelerator);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL(wrapped_token);
	ASSERT_NOT_NULL_RESULT(wrapped_handle->adapter_table->fpgaAssignToInterface,
			       FPGA_NOT_SUPPORTED);

	return wrapped_handle->adapter_table->fpgaAssignToInterface(
			wrapped_handle->opae_handle, wrapped_token->opae_token,
			host_interface, flags);
}

fpga_result fpgaReleaseFromInterface(fpga_handle fpga, fpga_token accelerator)
{
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(fpga);
	opae_wrapped_token  *wrapped_token  = opae_validate_wrapped_token(accelerator);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL(wrapped_token);
	ASSERT_NOT_NULL_RESULT(wrapped_handle->adapter_table->fpgaReleaseFromInterface,
			       FPGA_NOT_SUPPORTED);

	return wrapped_handle->adapter_table->fpgaReleaseFromInterface(
			wrapped_handle->opae_handle, wrapped_token->opae_token);
}

fpga_result fpgaReconfigureSlot(fpga_handle fpga, uint32_t slot,
				const uint8_t *bitstream, size_t bitstream_len,
				int flags)
{
	opae_wrapped_handle *wrapped_handle = opae_validate_wrapped_handle(fpga);

	ASSERT_NOT_NULL(wrapped_handle);
	ASSERT_NOT_NULL(bitstream);
	ASSERT_NOT_NULL_RESULT(wrapped_handle->adapter_table->fpgaReconfigureSlot,
			       FPGA_NOT_SUPPORTED);

	return wrapped_handle->adapter_table->fpgaReconfigureSlot(
			wrapped_handle->opae_handle, slot, bitstream,
			bitstream_len, flags);
}

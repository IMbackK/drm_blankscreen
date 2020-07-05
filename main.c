#include<stdio.h>
#include<stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include<xf86drmMode.h>
#include<xf86drm.h>
#include <linux/vt.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include"getfd.h"

#define DPMS_ID 2

int drm_get_device_connectors(int device, drmModeConnectorPtr** connectors)
{
	drmModeRes* resources;
	resources = drmModeGetResources(device);
	if(!resources) {
		perror("Can not get resources from drm device");
		return -1;
	}
	
	if(resources->count_connectors > 0) {
		*connectors = malloc(sizeof(drmModeConnectorPtr)*resources->count_connectors);
		if(!*connectors) {
			fprintf(stderr, "Out of memory!\n");
			return -2;
		}
		for(int i = 0; i < resources->count_connectors; ++i) {
			(*connectors)[i] = drmModeGetConnectorCurrent(device, resources->connectors[i]);
		}
		return resources->count_connectors;
	}
	
	else return -3;
}

void drm_free_device_connectors(drmModeConnectorPtr* connectors, int count_connectors)
{
	for(int i = 0; i < count_connectors; ++i) {
		drmModeFreeConnector(connectors[i]);
	}
	free(connectors);
}

int drm_get_connector_propertys(int device, drmModeConnectorPtr connector, drmModePropertyPtr** properties)
{
	*properties = malloc(sizeof(drmModePropertyRes*)*connector->count_props);
	if(!properties){
			fprintf(stderr, "Out of memory!\n");
			return -1;
	}
	
	for(int i = 0; i < connector->count_props; ++i) {
		(*properties)[i] = drmModeGetProperty(device, connector->props[i]);
	}
	
	return connector->count_props;
}

void drm_free_device_properties(drmModePropertyPtr* properties, int count_props)
{
	for(int i = 0; i < count_props; ++i) {
		drmModeFreeProperty(properties[i]);
	}
	free(properties);
}

int drm_open_device(char* fileName)
{
	int fd;
	fd = open(fileName, O_RDWR);
	if(fd < 0) {
		fprintf(stderr, "Can not open drm device %s: ", fileName);
		perror(NULL);
		return -1;
	}
	return fd;
}


int main(int argc, char** argv)
{
	printf("init\n");
	int device = drm_open_device("/dev/dri/card0");
	if(device < 0) return -1;
	
	printf("drm_get_device_connectors\n");
	drmModeConnectorPtr* connectors = NULL;
	int count_connectors = drm_get_device_connectors(device, &connectors);
	
	printf("count_connectors: %i %p\n", count_connectors, connectors);
	
	int vtFd = getfd(NULL);
	
	ioctl(vtFd,VT_ACTIVATE,2);
	
	sleep(1);
	
	for(int i = 0; i < count_connectors; ++i){
		drmModePropertyPtr* properties = NULL;
		
		printf("Connector: %u\n", connectors[i]->connector_id);
		int count_props = drm_get_connector_propertys(device, connectors[i], &properties);
		printf("count_props: %i %p\n", count_props, properties);
		drmModePropertyPtr drmProp = NULL;
		for(int j = 0; j < count_props; ++j){
			printf("	id: %u name: %s\n", properties[j]->prop_id, properties[j]->name);
			if(properties[j]->prop_id == DPMS_ID) drmProp = properties[j];
		}
		
		if(drmProp){
			drmSetMaster(device);
			perror(NULL);
			int ret = drmModeConnectorSetProperty(device, connectors[i]->connector_id, drmProp->prop_id, DRM_MODE_DPMS_OFF);
			printf("	ret: %i\n", ret);
			perror(NULL);
					drmDropMaster(device);
		}
		

		
		drm_free_device_properties(properties, count_props);
	}
	drm_free_device_connectors(connectors, count_connectors);
	
	return 0;
}

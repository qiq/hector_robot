// 
#include <config.h>

#include "Object.h"

char *Object::getValueSync(const char *name) {
	return NULL;
}

bool Object::setValueSync(const char *name, const char *value) {
	return false;
}

vector<string> *Object::listNamesSync() {
	return new vector<string>();
}

void Object::SaveCheckpointSync(const char *path, const char *id) {
}

void Object::RestoreCheckpointSync(const char *path, const char *id) {
}
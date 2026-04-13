#include "FSMState_min.h"

FSMState_min::FSMState_min(Model* model, std::string name) : DefaultNode(model, Util::TypeOf<FSMState_min>(), name) {
	std::string classname = Util::TypeOf<FSMState_min>();
}

void FSMState_min::setEntryActionExpression(std::string expression) {
	_entryActionExpression = expression;
}

std::string FSMState_min::getEntryActionExpression() const {
	return _entryActionExpression;
}

void FSMState_min::setExitActionExpression(std::string expression) {
	_exitActionExpression = expression;
}

std::string FSMState_min::getExitActionExpression() const {
	return _exitActionExpression;
}

PluginInformation* FSMState_min::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<FSMState_min>(), &FSMState_min::LoadInstance, &FSMState_min::NewInstance);
	info->setCategory("Network");
	info->setDescriptionHelp("FSM node/state with optional entry and exit action expressions.");
	info->setReceiveTransfer(true); // FSM nodes do not need to be connected from a source to a sink
	info->setSendTransfer(true); // FSM nodes do not need to be connected in a process flow
	return info;
}

ModelComponent* FSMState_min::LoadInstance(Model* model, PersistenceRecord *fields) {
	FSMState_min* component = new FSMState_min(model);
	component->_loadInstance(fields);
	return component;
}

ModelDataDefinition* FSMState_min::NewInstance(Model* model, std::string name) {
	return new FSMState_min(model, name);
}

bool FSMState_min::_loadInstance(PersistenceRecord *fields) {
	bool res = DefaultNode::_loadInstance(fields);
	if (res) {
		_entryActionExpression = fields->loadField("entryActionExpression", DEFAULT.entryActionExpression);
		_exitActionExpression = fields->loadField("exitActionExpression", DEFAULT.exitActionExpression);
	}
	return res;
}

void FSMState_min::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	DefaultNode::_saveInstance(fields, saveDefaultValues);
	fields->saveField("entryActionExpression", _entryActionExpression, DEFAULT.entryActionExpression, saveDefaultValues);
	fields->saveField("exitActionExpression", _exitActionExpression, DEFAULT.exitActionExpression, saveDefaultValues);
}

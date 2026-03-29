/*
 * ExtendedFSM.h
 *
 * Created on: 2025.2
 * Authors: André Rocco, Samantha Costa
 */

#ifndef EXTENDEDFSMCOMPONENT_H
#define EXTENDEDFSMCOMPONENT_H

#include "../../kernel/simulator/ModelComponent.h"
#include "FSMState.h"
#include "tinyexpr/tinyexpr.h"
#include <map>
#include <string>
#include <vector>

class ExtendedFSMComponent : public ModelComponent {
public:
    ExtendedFSMComponent(Model* model, std::string name = "");
    virtual ~ExtendedFSMComponent();

public:
    virtual std::string show();

public:
    static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
    static PluginInformation* GetPluginInformation();
    static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
    virtual bool _loadInstance(PersistenceRecord *fields);
    virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
    virtual bool _check(std::string* errorMessage);
    virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber);
    virtual void _createInternalAndAttachedData();

public:
    // Building the Graph
    FSMState* createState(std::string name, bool isFinal = false);
    void createVariable(std::string name, double val);
    
    // Accessors
    void setInitialState(FSMState* state);
    FSMState* getInitialState() const;
    FSMState* getCurrentState() const;

private:
    // Semantic Helpers
    bool evaluateGuard(std::string guard, Entity* entity);
    void executeSetActions(std::string actions);
    void executeOutputActions(Entity* entity, std::string actions);
    
    // Helper to parse actions string "var=val, var2=val2"
    std::map<std::string, std::string> parseActions(std::string actions);

private:
    // Core FSM Data
    std::map<std::string, FSMState*> _states;
    FSMState* _initialState = nullptr;
    FSMState* _currentState = nullptr;

    // Extended Data (Variables)
    std::map<std::string, double> _variables;
};

#endif /* EXTENDEDFSMCOMPONENT_H */

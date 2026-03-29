/*
 * FSMTransition.h
 *
 * Created on: 2025.2
 * Authors: André Rocco, Samantha Costa
 */

#ifndef FSMTRANSITION_H
#define FSMTRANSITION_H

#include "../../kernel/simulator/ModelDataDefinition.h"
#include "../../kernel/simulator/Plugin.h"
#include "FSMState.h"

class FSMTransition : public ModelDataDefinition {
public:
    FSMTransition(Model* model, std::string name = "");
    virtual ~FSMTransition();

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

public:
    void setGuard(std::string guard);
    std::string getGuard() const;
    
    void setSource(FSMState* source);
    FSMState* getSource() const;
    
    void setDestination(FSMState* destination);
    FSMState* getDestination() const;
    
    void setOutputActions(std::string outputActions);
    std::string getOutputActions() const;
    
    void setSetActions(std::string setActions);
    std::string getSetActions() const;
    
    void setPriority(int priority);
    int getPriority() const;
    
    bool isDefault() const;
    void setDefault(bool isDefault);

private:
    std::string _guard = "";
    FSMState* _source = nullptr;
    FSMState* _destination = nullptr;
    std::string _outputActions = ""; // Comma separated list of actions
    std::string _setActions = ""; // Comma separated list of actions
    int _priority = 0;
    bool _isDefault = false;
};

#endif /* FSMTRANSITION_H */

/*
 * FSMState.h
 *
 * Created on: 2025.2
 * Authors: André Rocco, Samantha Costa
 */

#ifndef FSMSTATE_H
#define FSMSTATE_H

#include "../../kernel/simulator/ModelDataDefinition.h"
#include "../../kernel/simulator/Plugin.h"
#include <list>

class FSMTransition; // Forward declaration

class FSMState : public ModelDataDefinition {
public:
    FSMState(Model* model, std::string name = "");
    virtual ~FSMState();

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
    void enter();
    void exit();
    std::list<FSMTransition*>* getTransitions() const;
    void setFinal(bool isFinal);
    bool isFinal() const;

private:
    std::list<FSMTransition*>* _transitions = new std::list<FSMTransition*>();
    bool _isFinal = false;
};

#endif /* FSMSTATE_H */

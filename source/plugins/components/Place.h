#ifndef PLACE_H
#define PLACE_H

#include <list>
#include <limits>
#include "DTBToken.h"
#include "../../kernel/simulator/ModelComponent.h"

class Place : public ModelComponent {
public:
    static const size_t MAX_CAPACITY = 30;

    Place(Model* model, std::string name = "");
    virtual ~Place() = default;

    DTBToken* getToken() {
        if (!_tokens.empty()) {
            return _tokens.front();
        }
        return nullptr;
    }

    void addToken(DTBToken* token);
    void removeToken(DTBToken* token);
    
    size_t getTokenCount() const;
    virtual std::string show() override;
    
    static PluginInformation* GetPluginInformation();
    static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
    static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

private:
    std::list<DTBToken*> _tokens;

protected:
    virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
};

#endif
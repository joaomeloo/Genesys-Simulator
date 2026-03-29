#ifndef DTBTOKEN_H
#define DTBTOKEN_H

#include <string>
#include <memory>
#include "../../kernel/simulator/ModelComponent.h"
 
class DTBToken : public ModelComponent {
public:
    DTBToken(Model* model, std::string name = "");

    virtual ~DTBToken() = default;

    void setValue(const std::string& value);

    std::string getValue() const;

    void setLifeTime(double time);

    double getLifeTime() const;

    virtual std::string show() override;

    static PluginInformation* GetPluginInformation();

    static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);

    static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;

private:
    std::string _value;

    double _lifeTime;
};

#endif

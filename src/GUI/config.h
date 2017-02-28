#ifndef CONFIG_H
#define CONFIG_H

class Configuration
{
public:
    explicit Configuration();
    
    double acceptableSolde(){return _acceptableSolde;}

private:
    double _acceptableSolde;
    
};

#endif // CONFIG_H

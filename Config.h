#ifndef CONFIG_H
#define CONFIG_H

#include <fstream>

class Config {
public:
    Config(const char *config_file);

    int getGroupSize() const { return group_size; }
    bool isSingleFailure() const { return single_failure; } 
    double getMsgDropProb() const { return msg_drop_prob; }
    double getInsertRate() const { return insert_rate; }
    short getPort() const { return port; }

private:
    void init(const char *config_file);

    int group_size;             
    int single_failure;			
    double msg_drop_prob;		
    double insert_rate;
    short port;
};

inline Config::Config(const char *config_file)
    : group_size(0)
    , single_failure(0)
    , msg_drop_prob(0)
    , insert_rate(.25)
    , port(8001)
{
    init(config_file);
}

inline void Config::init(const char *config_file) {
    std::ifstream is(config_file);

    std::string _;
    is >> _ >> group_size;
    is >> _ >> single_failure;

    int drop_msg = 0;
    is >> _ >> drop_msg;
    if (drop_msg) {
        is >> _ >> msg_drop_prob;
    } else {
        msg_drop_prob = 0;
    }
}

#endif /* CONFIG_H */

#pragma once

#include <string>
#include <optional>
#include <vector>
#include <Eigen/Core>
#include <fstream>

#include <jsonsimplecpp\node.hpp>
#include <jsonsimplecpp\parser.hpp>

#include <visitor.hpp>
#include <fileutils.hpp>
#include <forwardDecl.hpp>
#include <matlayer.hpp>
#include <basesolver.hpp>
#include <fitting.hpp>
#include <simulation.hpp>

namespace Data {

    enum class SolverMode{simulation, fitting, automatic};
    enum class FileFormat{CSV, JSON, automatic};

    //filepolicy is Json::Parser<ConfigVisitor>
    template<typename FilePolicy = Json::JsonParser<ConfigVisitor>>
    class Reader {
    private:
        SolverMode _smode;
        const std::string _filepath;
        std::shared_ptr<ConfigVisitor> _visitor;
        FilePolicy _policy;  //
        Json::JsonNode<ConfigVisitor>* _rootPtr;
    
        Json::JsonNode<ConfigVisitor>* parseFile() {
            _policy.parse();
            auto resPtr = const_cast<Json::JsonNode<ConfigVisitor>*>(_policy.getJsonTree());
            return resPtr;
        }
    
    public:
        Reader(const std::string& filepath) :
              _filepath{filepath},
              _visitor{std::make_shared<ConfigVisitor>()},
              _policy{_filepath, _visitor},
              _rootPtr{parseFile()}
        {
            if (_visitor->isSimulation()) _smode = SolverMode::fitting;
            else _smode = SolverMode::simulation;
        }
    
        SolverMode get_mode() {return _smode;}
    
        std::unique_ptr<BaseSolver> makeSolver() {
            return _visitor->makeSolver();
        }
    };

    class Importer {
        protected:

            Importer(const std::string& filepath);
            Importer(const std::string& filepath, SolverMode mode);
            const std::string& _filepath;

            virtual void setSolverMode() = 0;
            SolverMode _mode;

        public:
            virtual std::unique_ptr<BaseSolver> solverFromFile() = 0;
            virtual ~Importer() = default;
    };

    class INIimporter : public Importer {
        protected:
            void setSolverMode() override;
        
        public:
            std::unique_ptr<BaseSolver> solverFromFile() override;
            INIimporter(const std::string& filepath);
            INIimporter(const std::string& filepath, SolverMode mode);
    };

    class JSONimporter : public Importer {
        private:
            void setSolverMode() override;
            Reader<Json::JsonParser<ConfigVisitor>> _reader;
        
        public:
            std::unique_ptr<BaseSolver> solverFromFile() override;

            JSONimporter(const std::string& filepath);
            JSONimporter(const std::string& filepath, SolverMode mode);
    };

    class ImportManager {
        private:
            const std::string& _filepath;
            std::ifstream _fin;
            FileFormat _ftype;
            SolverMode _smode;
            
            void autoSetFileFormat();

        public:
            ImportManager(const std::string& filepath);
            ImportManager(const std::string& filepath, FileFormat);
            ImportManager(const std::string& filepath, SolverMode);
            ImportManager(const std::string& filepath, FileFormat, SolverMode);

            std::unique_ptr<Importer> makeImporter();
    };
}
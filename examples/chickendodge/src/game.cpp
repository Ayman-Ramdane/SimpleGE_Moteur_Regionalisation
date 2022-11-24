#include "chickendodge/pch/precomp.h"

#include "chickendodge/components.h"

static const SimpleGE::LocaleFiles Locales = {
    {"en", "locales/en.json"},
    {"fr", "locales/fr.json"},
};

constexpr auto TeamFile = "equipe.txt";
constexpr auto TeamDefaultString = "Coéquipiers";
constexpr auto LaunchScene = "scenes/play.json";
constexpr auto DefaultLang = "fr";
constexpr auto DefaultName = "Anonyme";

namespace ChickenDodge
{
  struct Config
  {
    static Config& Instance()
    {
      static Config instance;
      return instance;
    }

    static void SetName(std::string_view val) { Instance().alias = val; }

    static void SetOtherName(std::string_view val) { Instance().alias2 = val; }

    std::string alias{DefaultName};
    std::string alias2{DefaultName};
  };

  class Game : public SimpleGE::Game
  {
  public:
    Game()
    {
      ShowInstructions();
      PlayerComponent::SetNames({Config::Instance().alias, Config::Instance().alias2});
    }

  protected:
    [[nodiscard]] std::string_view GetLaunchScene() const override
    {
      return LaunchScene;
    }

    void SetupSystem() override
    {
      AddSystem<SimpleGE::PhysicSystem>();
      AddSystem<SimpleGE::LogicSystem>();
      AddSystem<SimpleGE::DisplaySystem>();
    }

    void RegisterComponents() const override { RegisterGameComponents(); }

  private:
    static void ShowInstructions()
    {
      fmt::print("{} ({}):\n", Config::Instance().alias, SimpleGE::Localisation::Get("greenPlayer"));
      fmt::print("  {}: {}\n", SimpleGE::Localisation::Get("CHAR_KEYS"), SimpleGE::Localisation::Get("MOVE"));
      fmt::print("  {}: {}\n", SimpleGE::Localisation::Get("SPACE_KEY"), SimpleGE::Localisation::Get("ATTACK"));

      fmt::print("{} ({}):\n", Config::Instance().alias2, SimpleGE::Localisation::Get("redPlayer"));
      fmt::print("  {}: {}\n", SimpleGE::Localisation::Get("ARROW_KEYS"), SimpleGE::Localisation::Get("MOVE"));
      fmt::print("  {}: {}\n", SimpleGE::Localisation::Get("ENTER_KEY"), SimpleGE::Localisation::Get("ATTACK"));
    }
  };

  static void ProcessCommandLine(gsl::span<char*> args)
  {
    SimpleGE::CommandLine::RegisterOption("-U", Config::SetName);
    SimpleGE::CommandLine::RegisterOption("-O", Config::SetOtherName);
    SimpleGE::CommandLine::Parse(args);
  }
} // namespace ChickenDodge

[[nodiscard]] static bool ValidateTeam()
{
  auto teamFile = SimpleGE::Resources::Get<SimpleGE::TextAsset>(TeamFile);
  if (teamFile == nullptr)
  {
    fmt::print(stderr, "Échec d'ouverture de {}\n", TeamFile);
    return false;
  }

  if ((std::string_view) *teamFile == TeamDefaultString)
  {
    fmt::print(stderr, "N'oubliez pas d'inscrire les membres de votre équipe dans le fichier data/equipe.txt!\n");
    return false;
  }

  return true;
}

[[nodiscard]] static bool SetupLocales()
{
  const char* lang = std::getenv("LANG");
  if (lang == nullptr)
  {
    lang = DefaultLang;
  }
  SimpleGE::Localisation::Init(Locales, std::string_view(lang, 2));

  return true;
}

int main(int argc, char* argv[])
{
  if (!ValidateTeam())
  {
    return -1;
  }

  if (!SetupLocales())
  {
    return -1;
  }

  ChickenDodge::ProcessCommandLine({argv, (size_t) argc});

  ChickenDodge::Game game;
  auto runResult = game.Run();
  return runResult == SimpleGE::Game::RunResult::Success ? 0 : 1;
}
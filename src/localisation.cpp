#include <iostream>
#include <regex>
#include <simplege/simplege.h>

using json = nlohmann::json;

namespace SimpleGE
{
  void Localisation::Init(const LocaleFiles& locales, std::string_view lang)
  {
    auto curLocale = locales.find(std::string(lang));
    if (curLocale == locales.end())
    {
      curLocale = locales.begin();
      fmt::print(stderr, "Config pour {} introuvable, on va utiliser {} à la place.\n", lang, curLocale->first);
    }

    auto content = Resources::Get<TextAsset>(curLocale->second);
    Ensures(content != nullptr);
    json::parse(content->Value()).get_to(Instance().localeStrings);
  }

  [[nodiscard]] std::string Localisation::GetImpl(std::string_view key, const LocaleContext& queryContext) const
  {
    LocaleContext mergedContext;
    std::set_union(queryContext.begin(), queryContext.end(), globalContext.begin(), globalContext.end(),
                   std::inserter(mergedContext, mergedContext.end()));

    auto localized = localeStrings.find(std::string(key));
    if (localized == localeStrings.end())
    {
      fmt::print(stderr, "Clé régionalisée {} introuvable.\n", key);
      return std::string(key);
    }

    // ***TODO***: Implémenter la substitution de clés

    auto message = localized->second;

    size_t start = message.find('{', 0);
    size_t end = message.find('}', 0);

    // Entrer dans la boucle si des accolades sont repérées dans le message
    while (start != std::string::npos && end != std::string::npos)
    {
      // Vérifier que les accolades sont dans le bon sens
      if (start >= end)
      {
        end = message.find('}', end + 1);
        // Stopper la boucle s'il n'y a pas d'autre accolade fermante
        if (end == std::string::npos)
        {
          break;
        }
      }

      // Récupérer la clé placée entre  accolades
      auto messageKey = message.substr(start + 1, end - start - 1);
      auto newMessage = mergedContext.find(messageKey)->second;

      // Effacer les accolades et substituer les termes
      message.erase(start, 1);
      message.erase(end - 1, 1);
      message = std::regex_replace(message, std::regex(messageKey), newMessage);

      // Incrémenter les index
      start = message.find('{', start + 1);
      end = message.find('}', end + 1);
    }

    return message;
  }
} // namespace SimpleGE
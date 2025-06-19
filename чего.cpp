#include <tgbot/tgbot.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>

using namespace TgBot;
using json = nlohmann::json;

// Структура для хранения состояния пользователя
enum class State {
    Idle,
    AwaitingName,
    AwaitingAge,
    AwaitingExperience,
    AwaitingSpecialty,
    AwaitingSkills
};

struct UserProfile {
    std::string name;
    int age = 0;
    std::string experience;
    std::vector<std::string> specialties;
    std::vector<std::string> skills;
    State state = State::Idle;
};

std::map<int64_t, UserProfile> userProfiles;

// Специальности и соответствующие навыки
const std::map<std::string, std::vector<std::string>> specialtySkills = {
    {"Программирование", {"C++", "Python", "Java", "JavaScript"}},
    {"Системное администрирование", {"Linux", "Windows Server", "Docker", "Kubernetes"}},
    {"Дизайн", {"Photoshop", "Figma", "Illustrator", "UI/UX"}}
};

// Кнопка отмены
const std::vector<std::vector<KeyboardButton::Ptr>> cancelButton = {
    {std::make_shared<KeyboardButton>("Отмена")}
};

// Сохранение профиля в JSON
void saveProfile(int64_t userId) {
    const UserProfile& profile = userProfiles[userId];
    json j;
    j["name"] = profile.name;
    j["age"] = profile.age;
    j["experience"] = profile.experience;
    j["specialties"] = profile.specialties;
    j["skills"] = profile.skills;

    std::ofstream file("user_" + std::to_string(userId) + ".json");
    file << j.dump(4);
    file.close();
}

// Очистка состояния пользователя
void resetUserState(int64_t userId) {
    userProfiles.erase(userId);
}

int main() {
    std::string token = "YOUR_BOT_TOKEN";
    Bot bot(token);

    // Обработчик команды /start
    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        resetUserState(message->chat->id);
        userProfiles[message->chat->id].state = State::AwaitingName;

        bot.getApi().sendMessage(
            message->chat->id,
            "Привет! Я помогу составить твое резюме. Сначала введи свое имя:",
            false, 0, std::make_shared<ReplyKeyboardMarkup>(cancelButton)
        );
    });

    // Обработчик текстовых сообщений
    bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
        int64_t userId = message->chat->id;
        std::string text = message->text;

        if (StringTools::startsWith(text, "/")) return;

        // Обработка отмены
        if (text == "Отмена") {
            resetUserState(userId);
            bot.getApi().sendMessage(userId, "Заполнение анкеты отменено.", true);
            return;
        }

        UserProfile& profile = userProfiles[userId];

        switch (profile.state) {
            case State::AwaitingName:
                profile.name = text;
                profile.state = State::AwaitingAge;
                bot.getApi().sendMessage(
                    userId, 
                    "Теперь введи свой возраст:",
                    false, 0, std::make_shared<ReplyKeyboardMarkup>(cancelButton)
                );
                break;

            case State::AwaitingAge:
                try {
                    profile.age = std::stoi(text);
                    if (profile.age <= 0 || profile.age > 100) throw std::exception();

                    // Кнопки для выбора опыта
                    auto keyboard = std::make_shared<InlineKeyboardMarkup>();
                    std::vector<InlineKeyboardButton::Ptr> row;
                    for (const std::string& exp : {"Нет опыта", "Менее 1 года", "1-3 года", "Более 3 лет"}) {
                        auto button = std::make_shared<InlineKeyboardButton>();
                        button->text = exp;
                        button->callbackData = exp;
                        row.push_back(button);
                        if (row.size() == 2) {
                            keyboard->inlineKeyboard.push_back(row);
                            row.clear();
                        }
                    }
                    if (!row.empty()) keyboard->inlineKeyboard.push_back(row);

                    profile.state = State::AwaitingExperience;
                    bot.getApi().sendMessage(
                        userId, 
                        "Выбери опыт работы:", 
                        false, 0, keyboard
                    );
                } catch (...) {
                    bot.getApi().sendMessage(userId, "Некорректный возраст! Попробуй еще раз:");
                }
                break;

            // Обработка остальных состояний происходит через callback-и
            default:
                bot.getApi().sendMessage(userId, "Используй кнопки для выбора.");
        }
    });

    // Обработчик callback-ов (опыт, специальности, навыки)
    bot.getEvents().onCallbackQuery([&bot](CallbackQuery::Ptr query) {
        int64_t userId = query->message->chat->id;
        UserProfile& profile = userProfiles[userId];
        std::string data = query->data;

        if (profile.state == State::AwaitingExperience) {
            profile.experience = data;
            profile.state = State::AwaitingSpecialty;

            // Клавиатура для выбора специальностей
            auto keyboard = std::make_shared<InlineKeyboardMarkup>();
            for (const auto& specialty : specialtySkills) {
                std::vector<InlineKeyboardButton::Ptr> row;
                auto button = std::make_shared<InlineKeyboardButton>();
                button->text = specialty.first;
                button->callbackData = "spec_" + specialty.first;
                row.push_back(button);
                keyboard->inlineKeyboard.push_back(row);
            }
            auto doneButton = std::make_shared<InlineKeyboardButton>();
            doneButton->text = "Готово";
            doneButton->callbackData = "spec_done";
            keyboard->inlineKeyboard.push_back({doneButton});

            bot.getApi().editMessageText(
                "Выбери специальности (можно несколько):",
                query->message->chat->id,
                query->message->messageId,
                "",  // parseMode
                false,
                keyboard
            );
        }
        else if (profile.state == State::AwaitingSpecialty && data.find("spec_") == 0) {
            std::string specialty = data.substr(5);
            auto it = std::find(profile.specialties.begin(), profile.specialties.end(), specialty);
            if (it == profile.specialties.end()) {
                profile.specialties.push_back(specialty);
            } else {
                profile.specialties.erase(it);
            }
            // Обновляем сообщение с галочками
            auto keyboard = std::make_shared<InlineKeyboardMarkup>();
            for (const auto& sp : specialtySkills) {
                std::vector<InlineKeyboardButton::Ptr> row;
                auto button = std::make_shared<InlineKeyboardButton>();
                button->text = sp.first + (std::find(profile.specialties.begin(), profile.specialties.end(), sp.first) != profile.specialties.end() ? " ✓" : "");
                button->callbackData = "spec_" + sp.first;
                row.push_back(button);
                keyboard->inlineKeyboard.push_back(row);
            }
            auto doneButton = std::make_shared<InlineKeyboardButton>();
            doneButton->text = "Готово";
            doneButton->callbackData = "spec_done";
            keyboard->inlineKeyboard.push_back({doneButton});

            bot.getApi().editMessageReplyMarkup(
                userId,
                query->message->messageId,
                keyboard
            );
        }
        else if (profile.state == State::AwaitingSpecialty && data == "spec_done") {
            if (profile.specialties.empty()) {
                bot.getApi().sendMessage(userId, "Выбери хотя бы одну специальность!");
                return;
            }
            profile.state = State::AwaitingSkills;
            
            // Клавиатура для выбора навыков
            auto keyboard = std::make_shared<InlineKeyboardMarkup>();
            std::set<std::string> allSkills;
            for (const auto& spec : profile.specialties) {
                for (const auto& skill : specialtySkills.at(spec)) {
                    allSkills.insert(skill);
                }
            }
            for (const auto& skill : allSkills) {
                std::vector<InlineKeyboardButton::Ptr> row;
                auto button = std::make_shared<InlineKeyboardButton>();
                button->text = skill;
                button->callbackData = "skill_" + skill;
                row.push_back(button);
                keyboard->inlineKeyboard.push_back(row);
            }
            auto doneButton = std::make_shared<InlineKeyboardButton>();
            doneButton->text = "Завершить";
            doneButton->callbackData = "skills_done";
            keyboard->inlineKeyboard.push_back({doneButton});

            bot.getApi().editMessageText(
                "Выбери навыки (можно несколько):",
                userId,
                query->message->messageId,
                "", false, keyboard
            );
        }
        else if (profile.state == State::AwaitingSkills && data.find("skill_") == 0) {
            std::string skill = data.substr(6);
            auto it = std::find(profile.skills.begin(), profile.skills.end(), skill);
            if (it == profile.skills.end()) {
                profile.skills.push_back(skill);
            } else {
                profile.skills.erase(it);
            }
            // Обновляем сообщение
            auto keyboard = std::make_shared<InlineKeyboardMarkup>();
            std::set<std::string> allSkills;
            for (const auto& spec : profile.specialties) {
                for (const auto& sk : specialtySkills.at(spec)) {
                    allSkills.insert(sk);
                }
            }
            for (const auto& sk : allSkills) {
                std::vector<InlineKeyboardButton::Ptr> row;
                auto button = std::make_shared<InlineKeyboardButton>();
                button->text = sk + (std::find(profile.skills.begin(), profile.skills.end(), sk) != profile.skills.end() ? " ✓" : "");
                button->callbackData = "skill_" + sk;
                row.push_back(button);
                keyboard->inlineKeyboard.push_back(row);
            }
            auto doneButton = std::make_shared<InlineKeyboardButton>();
            doneButton->text = "Завершить";
            doneButton->callbackData = "skills_done";
            keyboard->inlineKeyboard.push_back({doneButton});

            bot.getApi().editMessageReplyMarkup(
                userId,
                query->message->messageId,
                keyboard
            );
        }
        else if (profile.state == State::AwaitingSkills && data == "skills_done") {
            saveProfile(userId);
            bot.getApi().sendMessage(
                userId,
                "Анкета сохранена! Файл: user_" + std::to_string(userId) + ".json",
                true
            );
            resetUserState(userId);
        }
    });

    // Обработчик неизвестных команд
    bot.getEvents().onUnknownCommand([&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Неизвестная команда. Используй /start для начала.");
    });

    try {
        std::cout << "Bot username: " << bot.getApi().getMe()->username << "\n";
        bot.getApi().deleteWebhook();
        TgLongPoll longPoll(bot);
        while (true) {
            longPoll.start();
        }
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }

    return 0;
}

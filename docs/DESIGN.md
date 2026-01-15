# Rick and Morty Viewer - Design Document

## Overview

A C++ application that consumes the Rick and Morty REST API to display episodes and their characters. The architecture emphasizes separation of concerns, using the Observer pattern to decouple the backend data layer from the UI layer.

### Goals

- Display a list of all episodes from the API
- Show characters for each episode, sorted alphabetically by name
- Fetch data on-demand to minimize unnecessary network requests
- Decouple backend from frontend to allow UI framework replacement
- Use libcurl for HTTP requests and nlohmann/json for JSON parsing

### Dependencies

| Library | Purpose |
|---------|---------|
| libcurl | HTTP client for REST API calls |
| nlohmann/json | JSON parsing and serialization |
| Qt6 (replaceable) | UI framework |
| Qt6 Quick/QML | Declarative UI with animations |
| Qt6 QuickControls2 | Styled UI controls |

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│                              Application                                │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │                        UI Layer (Qt)                            │   │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐ │   │
│  │  │ MainWindow  │  │ EpisodeList │  │ CharacterDetailWidget   │ │   │
│  │  │             │  │   Widget    │  │                         │ │   │
│  │  └─────────────┘  └─────────────┘  └─────────────────────────┘ │   │
│  │                          │                                      │   │
│  │                          │ implements IObserver                 │   │
│  └──────────────────────────┼──────────────────────────────────────┘   │
│                             │                                          │
│  ┌──────────────────────────┼──────────────────────────────────────┐   │
│  │                    Core Layer (Backend)                         │   │
│  │                          │                                      │   │
│  │  ┌───────────────────────▼───────────────────────────────────┐ │   │
│  │  │                   DataStore                               │ │   │
│  │  │  - Episodes cache                                         │ │   │
│  │  │  - Characters cache                                       │ │   │
│  │  │  - Locations cache                                        │ │   │
│  │  │  - Observer management                                    │ │   │
│  │  └───────────────────────┬───────────────────────────────────┘ │   │
│  │                          │                                      │   │
│  │  ┌───────────────────────▼───────────────────────────────────┐ │   │
│  │  │                   ApiClient                               │ │   │
│  │  │  - HTTP requests via libcurl                              │ │   │
│  │  │  - JSON parsing via nlohmann/json                         │ │   │
│  │  │  - Pagination handling                                    │ │   │
│  │  └───────────────────────────────────────────────────────────┘ │   │
│  │                                                                 │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

### Layer Responsibilities

| Layer | Responsibility |
|-------|----------------|
| **UI Layer** | Presentation, user interaction, observes data changes |
| **Core Layer** | Business logic, data management, API communication |

---

## Observer Pattern

The Observer pattern decouples the backend from the frontend, allowing any UI framework to be used.

### Interface Definitions

```cpp
// Forward declarations
struct Episode;
struct Character;

// Event types for granular notifications
enum class DataEvent {
    EpisodesLoaded,
    EpisodeUpdated,
    CharactersLoaded,
    CharacterUpdated,
    LoadingStarted,
    LoadingFinished,
    ErrorOccurred
};

// Observer interface - UI components implement this
class IDataObserver {
public:
    virtual ~IDataObserver() = default;

    virtual void onEpisodesLoaded(const std::vector<Episode>& episodes) = 0;
    virtual void onCharactersLoaded(int episodeId, const std::vector<Character>& characters) = 0;
    virtual void onLoadingStateChanged(bool isLoading) = 0;
    virtual void onError(const std::string& message) = 0;
};

// Subject interface - DataStore implements this
class IDataSubject {
public:
    virtual ~IDataSubject() = default;

    virtual void addObserver(IDataObserver* observer) = 0;
    virtual void removeObserver(IDataObserver* observer) = 0;

protected:
    virtual void notifyEpisodesLoaded(const std::vector<Episode>& episodes) = 0;
    virtual void notifyCharactersLoaded(int episodeId, const std::vector<Character>& characters) = 0;
    virtual void notifyLoadingStateChanged(bool isLoading) = 0;
    virtual void notifyError(const std::string& message) = 0;
};
```

### Observer Flow

```
┌──────────────┐         ┌───────────────┐         ┌─────────────┐
│   UI Layer   │         │   DataStore   │         │  ApiClient  │
│  (Observer)  │         │   (Subject)   │         │             │
└──────┬───────┘         └───────┬───────┘         └──────┬──────┘
       │                         │                        │
       │  1. Request episodes    │                        │
       │────────────────────────►│                        │
       │                         │  2. Fetch from API     │
       │                         │───────────────────────►│
       │                         │                        │
       │                         │  3. Return JSON        │
       │                         │◄───────────────────────│
       │                         │                        │
       │  4. notifyEpisodesLoaded│                        │
       │◄────────────────────────│                        │
       │                         │                        │
       │  5. Update UI           │                        │
       │                         │                        │
```

---

## Data Models

### Enumerations

```cpp
enum class CharacterStatus {
    Alive,
    Dead,
    Unknown
};

enum class Gender {
    Female,
    Male,
    Genderless,
    Unknown
};
```

### Core Entities

```cpp
struct LocationReference {
    std::string name;
    std::string url;
    int id;  // Extracted from URL for lazy loading
};

struct Character {
    int id;
    std::string name;
    CharacterStatus status;
    std::string species;
    std::string type;
    Gender gender;
    LocationReference origin;
    LocationReference location;
    std::string imageUrl;
    std::vector<int> episodeIds;  // Extracted from URLs
    std::string url;
    std::string created;

    // For alphabetical sorting
    bool operator<(const Character& other) const {
        return name < other.name;
    }
};

struct Location {
    int id;
    std::string name;
    std::string type;
    std::string dimension;
    std::vector<int> residentIds;  // Extracted from URLs
    std::string url;
    std::string created;
};

struct Episode {
    int id;
    std::string name;
    std::string airDate;
    std::string episodeCode;  // e.g., "S01E01"
    std::vector<int> characterIds;  // Extracted from URLs
    std::string url;
    std::string created;

    // Parsed from episodeCode
    int season;
    int episodeNumber;
};
```

### Pagination Info

```cpp
struct PaginationInfo {
    int count;
    int pages;
    std::optional<std::string> next;
    std::optional<std::string> prev;
};
```

### JSON Serialization

Using nlohmann/json with custom from_json functions:

```cpp
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Helper to extract ID from URL
inline int extractIdFromUrl(const std::string& url) {
    auto pos = url.rfind('/');
    if (pos != std::string::npos) {
        return std::stoi(url.substr(pos + 1));
    }
    return -1;
}

// Character deserialization
void from_json(const json& j, Character& c) {
    j.at("id").get_to(c.id);
    j.at("name").get_to(c.name);
    j.at("species").get_to(c.species);
    j.at("type").get_to(c.type);
    j.at("image").get_to(c.imageUrl);
    j.at("url").get_to(c.url);
    j.at("created").get_to(c.created);

    // Parse status
    std::string status = j.at("status").get<std::string>();
    if (status == "Alive") c.status = CharacterStatus::Alive;
    else if (status == "Dead") c.status = CharacterStatus::Dead;
    else c.status = CharacterStatus::Unknown;

    // Parse gender
    std::string gender = j.at("gender").get<std::string>();
    if (gender == "Female") c.gender = Gender::Female;
    else if (gender == "Male") c.gender = Gender::Male;
    else if (gender == "Genderless") c.gender = Gender::Genderless;
    else c.gender = Gender::Unknown;

    // Parse location references
    c.origin.name = j.at("origin").at("name").get<std::string>();
    c.origin.url = j.at("origin").at("url").get<std::string>();
    c.origin.id = extractIdFromUrl(c.origin.url);

    c.location.name = j.at("location").at("name").get<std::string>();
    c.location.url = j.at("location").at("url").get<std::string>();
    c.location.id = extractIdFromUrl(c.location.url);

    // Extract episode IDs from URLs
    for (const auto& epUrl : j.at("episode")) {
        c.episodeIds.push_back(extractIdFromUrl(epUrl.get<std::string>()));
    }
}

// Episode deserialization
void from_json(const json& j, Episode& e) {
    j.at("id").get_to(e.id);
    j.at("name").get_to(e.name);
    j.at("air_date").get_to(e.airDate);
    j.at("episode").get_to(e.episodeCode);
    j.at("url").get_to(e.url);
    j.at("created").get_to(e.created);

    // Parse episode code (S01E01 format)
    if (e.episodeCode.length() >= 6) {
        e.season = std::stoi(e.episodeCode.substr(1, 2));
        e.episodeNumber = std::stoi(e.episodeCode.substr(4, 2));
    }

    // Extract character IDs from URLs
    for (const auto& charUrl : j.at("characters")) {
        e.characterIds.push_back(extractIdFromUrl(charUrl.get<std::string>()));
    }
}
```

---

## Component Design

### ApiClient

Handles HTTP communication using libcurl.

```cpp
class ApiClient {
public:
    ApiClient();
    ~ApiClient();

    // Fetch all episodes (handles pagination internally)
    std::vector<Episode> fetchAllEpisodes();

    // Fetch single episode
    std::optional<Episode> fetchEpisode(int id);

    // Fetch multiple characters by IDs (batch request)
    std::vector<Character> fetchCharacters(const std::vector<int>& ids);

    // Fetch single character
    std::optional<Character> fetchCharacter(int id);

    // Fetch location
    std::optional<Location> fetchLocation(int id);

private:
    static constexpr const char* BASE_URL = "https://rickandmortyapi.com/api";

    // Perform HTTP GET request
    std::string httpGet(const std::string& url);

    // Parse paginated response
    template<typename T>
    std::vector<T> fetchAllPaginated(const std::string& endpoint);

    CURL* curl_;
};
```

### HTTP Implementation

```cpp
std::string ApiClient::httpGet(const std::string& url) {
    std::string response;

    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, std::string* data) {
        data->append(ptr, size * nmemb);
        return size * nmemb;
    });
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl_);
    if (res != CURLE_OK) {
        throw std::runtime_error("HTTP request failed: " + std::string(curl_easy_strerror(res)));
    }

    return response;
}

std::vector<Character> ApiClient::fetchCharacters(const std::vector<int>& ids) {
    if (ids.empty()) return {};

    // Build comma-separated ID list for batch request
    std::string idList;
    for (size_t i = 0; i < ids.size(); ++i) {
        if (i > 0) idList += ",";
        idList += std::to_string(ids[i]);
    }

    std::string url = std::string(BASE_URL) + "/character/" + idList;
    std::string response = httpGet(url);

    json j = json::parse(response);

    // Single character returns object, multiple returns array
    if (j.is_array()) {
        return j.get<std::vector<Character>>();
    } else {
        return { j.get<Character>() };
    }
}
```

### DataStore

Central data management with caching and observer notifications.

```cpp
class DataStore : public IDataSubject {
public:
    DataStore(std::unique_ptr<ApiClient> apiClient);

    // Observer management
    void addObserver(IDataObserver* observer) override;
    void removeObserver(IDataObserver* observer) override;

    // Data access (triggers fetch if not cached)
    void loadAllEpisodes();
    void loadCharactersForEpisode(int episodeId);

    // Cached data access (no fetch)
    const std::vector<Episode>& getEpisodes() const;
    std::vector<Character> getCharactersForEpisode(int episodeId) const;
    std::optional<Character> getCharacter(int id) const;

    // Check if data is loaded
    bool areEpisodesLoaded() const;
    bool areCharactersLoadedForEpisode(int episodeId) const;

private:
    std::unique_ptr<ApiClient> apiClient_;
    std::vector<IDataObserver*> observers_;

    // Caches
    std::vector<Episode> episodes_;
    std::unordered_map<int, Character> characterCache_;
    std::unordered_set<int> loadedEpisodeCharacters_;

    bool episodesLoaded_ = false;

    // Notification helpers
    void notifyEpisodesLoaded(const std::vector<Episode>& episodes) override;
    void notifyCharactersLoaded(int episodeId, const std::vector<Character>& characters) override;
    void notifyLoadingStateChanged(bool isLoading) override;
    void notifyError(const std::string& message) override;
};
```

### DataStore Implementation

```cpp
void DataStore::loadCharactersForEpisode(int episodeId) {
    // Check if already loaded
    if (loadedEpisodeCharacters_.count(episodeId)) {
        auto characters = getCharactersForEpisode(episodeId);
        notifyCharactersLoaded(episodeId, characters);
        return;
    }

    notifyLoadingStateChanged(true);

    try {
        // Find episode
        auto it = std::find_if(episodes_.begin(), episodes_.end(),
            [episodeId](const Episode& e) { return e.id == episodeId; });

        if (it == episodes_.end()) {
            throw std::runtime_error("Episode not found: " + std::to_string(episodeId));
        }

        // Determine which characters need fetching
        std::vector<int> toFetch;
        for (int charId : it->characterIds) {
            if (characterCache_.find(charId) == characterCache_.end()) {
                toFetch.push_back(charId);
            }
        }

        // Fetch missing characters
        if (!toFetch.empty()) {
            auto fetched = apiClient_->fetchCharacters(toFetch);
            for (auto& c : fetched) {
                characterCache_[c.id] = std::move(c);
            }
        }

        loadedEpisodeCharacters_.insert(episodeId);

        // Get sorted characters
        auto characters = getCharactersForEpisode(episodeId);

        notifyLoadingStateChanged(false);
        notifyCharactersLoaded(episodeId, characters);

    } catch (const std::exception& e) {
        notifyLoadingStateChanged(false);
        notifyError(e.what());
    }
}

std::vector<Character> DataStore::getCharactersForEpisode(int episodeId) const {
    std::vector<Character> result;

    auto it = std::find_if(episodes_.begin(), episodes_.end(),
        [episodeId](const Episode& e) { return e.id == episodeId; });

    if (it != episodes_.end()) {
        for (int charId : it->characterIds) {
            auto charIt = characterCache_.find(charId);
            if (charIt != characterCache_.end()) {
                result.push_back(charIt->second);
            }
        }
    }

    // Sort alphabetically by name
    std::sort(result.begin(), result.end());

    return result;
}
```

---

## Directory Structure

```
project/
├── CMakeLists.txt
├── docs/
│   ├── API.md
│   └── DESIGN.md
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── CMakeLists.txt
│   │   ├── ApiClient.h
│   │   ├── ApiClient.cpp
│   │   ├── DataStore.h
│   │   ├── DataStore.cpp
│   │   ├── Models.h
│   │   └── Observer.h
│   └── ui/
│       ├── CMakeLists.txt
│       ├── MainWindow.h
│       ├── MainWindow.cpp
│       ├── EpisodeListWidget.h
│       ├── EpisodeListWidget.cpp
│       ├── CharacterListWidget.h
│       └── CharacterListWidget.cpp
└── tests/
    ├── CMakeLists.txt
    └── ...
```

---

## UI Layer (Qt Implementation)

The UI layer implements `IDataObserver` and can be replaced with another framework.

### MainWindow

```cpp
class MainWindow : public QMainWindow, public IDataObserver {
    Q_OBJECT

public:
    explicit MainWindow(DataStore* dataStore, QWidget* parent = nullptr);
    ~MainWindow();

    // IDataObserver implementation
    void onEpisodesLoaded(const std::vector<Episode>& episodes) override;
    void onCharactersLoaded(int episodeId, const std::vector<Character>& characters) override;
    void onLoadingStateChanged(bool isLoading) override;
    void onError(const std::string& message) override;

private slots:
    void onEpisodeSelected(int episodeId);

private:
    DataStore* dataStore_;
    EpisodeListWidget* episodeList_;
    CharacterListWidget* characterList_;
    QProgressBar* loadingIndicator_;
};
```

### Thread Safety for Qt

Since API calls are blocking, use Qt's threading:

```cpp
void MainWindow::onEpisodeSelected(int episodeId) {
    // Run fetch in background thread
    QFuture<void> future = QtConcurrent::run([this, episodeId]() {
        dataStore_->loadCharactersForEpisode(episodeId);
    });
}

// Observer callbacks must be thread-safe
void MainWindow::onCharactersLoaded(int episodeId, const std::vector<Character>& characters) {
    // Marshal to UI thread
    QMetaObject::invokeMethod(this, [this, episodeId, characters]() {
        characterList_->setCharacters(characters);
    }, Qt::QueuedConnection);
}
```

---

## Visual Design

> **See [UI_DESIGN.md](UI_DESIGN.md)** for comprehensive visual design specifications including:
> - Color palette and theme colors
> - Typography and font sizing
> - Responsive breakpoints and layout strategies
> - Component specifications (drawer, cards, hamburger button)
> - Character detail modal design
> - Animation specifications
> - UI improvement recommendations

---

## Updated Directory Structure

```
project/
├── CMakeLists.txt
├── docs/
│   ├── API.md
│   └── DESIGN.md
├── resources/
│   ├── resources.qrc
│   ├── images/
│   │   ├── particle.png
│   │   ├── plasma_gradient.png
│   │   └── logo.png
│   └── qml/
│       ├── Main.qml
│       ├── Theme.qml
│       ├── qmldir
│       ├── components/
│       │   ├── PortalEffect.qml
│       │   ├── PortalSpinner.qml
│       │   ├── CharacterCard.qml
│       │   ├── EpisodeDelegate.qml
│       │   └── EpisodeListView.qml
│       └── popups/
│           └── CharacterDetailPopup.qml
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── ApiClient.h
│   │   ├── ApiClient.cpp
│   │   ├── DataStore.h
│   │   ├── DataStore.cpp
│   │   ├── Models.h
│   │   └── Observer.h
│   └── ui/
│       ├── QmlBridge.h          # Exposes C++ to QML
│       ├── QmlBridge.cpp
│       ├── EpisodeModel.h       # QAbstractListModel
│       ├── EpisodeModel.cpp
│       ├── CharacterModel.h     # QAbstractListModel
│       └── CharacterModel.cpp
└── tests/
    └── ...
```

---

## QML-C++ Integration

### QML Bridge

Exposes the backend DataStore to QML.

```cpp
// QmlBridge.h
#pragma once
#include <QObject>
#include <QQmlEngine>
#include "core/DataStore.h"

class QmlBridge : public QObject, public IDataObserver {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorOccurred)

public:
    explicit QmlBridge(DataStore* dataStore, QObject* parent = nullptr);

    bool isLoading() const { return isLoading_; }
    QString errorMessage() const { return errorMessage_; }

    Q_INVOKABLE void loadEpisodes();
    Q_INVOKABLE void loadCharactersForEpisode(int episodeId);

    // IDataObserver implementation
    void onEpisodesLoaded(const std::vector<Episode>& episodes) override;
    void onCharactersLoaded(int episodeId, const std::vector<Character>& characters) override;
    void onLoadingStateChanged(bool isLoading) override;
    void onError(const std::string& message) override;

signals:
    void episodesReady();
    void charactersReady(int episodeId);
    void loadingChanged();
    void errorOccurred(const QString& message);

private:
    DataStore* dataStore_;
    bool isLoading_ = false;
    QString errorMessage_;
};
```

### Model Classes for QML

```cpp
// EpisodeModel.h
#pragma once
#include <QAbstractListModel>
#include "core/Models.h"

class EpisodeModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        EpisodeCodeRole,
        AirDateRole,
        CharacterIdsRole,
        SeasonRole,
        EpisodeNumberRole
    };

    explicit EpisodeModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void setEpisodes(const std::vector<Episode>& episodes);

private:
    std::vector<Episode> episodes_;
};

// CharacterModel.h
#pragma once
#include <QAbstractListModel>
#include "core/Models.h"

class CharacterModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        StatusRole,
        SpeciesRole,
        TypeRole,
        GenderRole,
        OriginRole,
        LocationRole,
        ImageUrlRole
    };

    explicit CharacterModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void setCharacters(const std::vector<Character>& characters);

private:
    std::vector<Character> characters_;
};
```

---

## Data Flow

### Application Startup

```
1. Application starts
2. MainWindow registers as observer with DataStore
3. MainWindow calls dataStore->loadAllEpisodes()
4. DataStore fetches all episodes (handling pagination)
5. DataStore caches episodes
6. DataStore notifies observers via onEpisodesLoaded()
7. MainWindow populates episode list
```

### Episode Selection

```
1. User selects episode in UI
2. MainWindow calls dataStore->loadCharactersForEpisode(id)
3. DataStore checks cache for characters
4. DataStore fetches missing characters via batch API call
5. DataStore caches new characters
6. DataStore notifies observers via onCharactersLoaded()
7. MainWindow displays characters sorted alphabetically
```

### Sequence Diagram

```
┌────────┐       ┌───────────┐       ┌───────────┐       ┌──────────┐
│  User  │       │ MainWindow│       │ DataStore │       │ApiClient │
└───┬────┘       └─────┬─────┘       └─────┬─────┘       └────┬─────┘
    │                  │                   │                  │
    │ Select Episode   │                   │                  │
    │─────────────────►│                   │                  │
    │                  │                   │                  │
    │                  │ loadCharacters(5) │                  │
    │                  │──────────────────►│                  │
    │                  │                   │                  │
    │                  │                   │ fetchCharacters([1,2,3])
    │                  │                   │─────────────────►│
    │                  │                   │                  │
    │                  │                   │   characters[]   │
    │                  │                   │◄─────────────────│
    │                  │                   │                  │
    │                  │ onCharactersLoaded│                  │
    │                  │◄──────────────────│                  │
    │                  │                   │                  │
    │  Display chars   │                   │                  │
    │◄─────────────────│                   │                  │
    │  (sorted A-Z)    │                   │                  │
```

---

## Caching Strategy

### Cache Levels

| Data | Cache Strategy | Invalidation |
|------|----------------|--------------|
| Episodes | Load all on startup, keep for session | Manual refresh |
| Characters | Load on-demand per episode, accumulate | Manual refresh |
| Locations | Load on-demand when character detail viewed | Manual refresh |
| Images | Optional: filesystem cache | LRU eviction |

### Memory Considerations

With ~826 characters and ~51 episodes:
- Episode data: ~51 * ~200 bytes = ~10 KB
- Character data: ~826 * ~500 bytes = ~400 KB
- Total in-memory: < 1 MB (acceptable)

---

## Error Handling

```cpp
enum class ApiError {
    NetworkError,
    ParseError,
    NotFound,
    RateLimited,
    Unknown
};

class ApiException : public std::exception {
public:
    ApiException(ApiError type, const std::string& message);

    ApiError type() const { return type_; }
    const char* what() const noexcept override { return message_.c_str(); }

private:
    ApiError type_;
    std::string message_;
};
```

### Error Recovery

| Error | Handling |
|-------|----------|
| Network timeout | Retry with exponential backoff (max 3 attempts) |
| 404 Not Found | Log and skip, notify user |
| Parse error | Log full response, notify user |
| Rate limiting | Queue requests, add delay |

---

## Future Considerations

### UI Framework Replacement

To replace Qt with another framework (e.g., GTK, ncurses, web):

1. Implement `IDataObserver` in new UI classes
2. Replace Qt widgets with equivalent components
3. Handle thread marshaling for UI updates
4. No changes required to core layer

### Potential Enhancements

- **Async API**: Replace blocking curl with async (curl_multi or separate threads)
- **Persistent cache**: SQLite or filesystem cache for offline support
- **Search/filter**: Local filtering of cached data
- **Image loading**: Async image downloads with placeholder

---

## Responsive Design

> **See [UI_DESIGN.md](UI_DESIGN.md)** for comprehensive responsive design specifications including:
> - Breakpoints and layout strategies
> - Floating drawer behavior matrix
> - Component sizing rules per breakpoint
> - Animation specifications

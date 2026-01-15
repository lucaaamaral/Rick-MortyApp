# Rick and Morty API Documentation

This document describes the Rick and Morty API, including schemas, usage patterns, and entity relationships.

**Source:** https://rickandmortyapi.com/documentation/

## Overview

| Property | Value |
|----------|-------|
| Base URL | `https://rickandmortyapi.com/api` |
| GraphQL Endpoint | `https://rickandmortyapi.com/graphql` |
| Protocol | HTTPS |
| Method | GET (REST) |
| Response Format | JSON |
| Authentication | None required |
| Rate Limiting | Not specified |

## Pagination

All collection endpoints return paginated responses with up to **20 items per page**.

### Info Object Schema

Every paginated response includes an `info` object:

```json
{
  "info": {
    "count": 826,
    "pages": 42,
    "next": "https://rickandmortyapi.com/api/character?page=2",
    "prev": null
  },
  "results": [...]
}
```

| Field | Type | Description |
|-------|------|-------------|
| `count` | `int` | Total number of items in the resource |
| `pages` | `int` | Total number of pages |
| `next` | `string \| null` | URL to the next page (null if last page) |
| `prev` | `string \| null` | URL to the previous page (null if first page) |

**Usage:** Append `?page=<number>` to navigate pages.

---

## Schemas

### Character Schema

**Total Characters:** 826

```typescript
interface Character {
  id: number;
  name: string;
  status: "Alive" | "Dead" | "unknown";
  species: string;
  type: string;
  gender: "Female" | "Male" | "Genderless" | "unknown";
  origin: LocationReference;
  location: LocationReference;
  image: string;
  episode: string[];
  url: string;
  created: string;
}

interface LocationReference {
  name: string;
  url: string;
}
```

| Field | Type | Description |
|-------|------|-------------|
| `id` | `int` | Unique identifier |
| `name` | `string` | Character's full name |
| `status` | `string` | Life status: `Alive`, `Dead`, or `unknown` |
| `species` | `string` | Species (e.g., "Human", "Alien") |
| `type` | `string` | Subspecies or type (can be empty) |
| `gender` | `string` | Gender: `Female`, `Male`, `Genderless`, or `unknown` |
| `origin` | `object` | Origin location with `name` and `url` |
| `location` | `object` | Last known location with `name` and `url` |
| `image` | `string` | URL to 300x300px avatar image |
| `episode` | `string[]` | Array of episode URLs where character appears |
| `url` | `string` | URL to character's own endpoint |
| `created` | `string` | ISO 8601 timestamp of creation in database |

### Location Schema

**Total Locations:** 126

```typescript
interface Location {
  id: number;
  name: string;
  type: string;
  dimension: string;
  residents: string[];
  url: string;
  created: string;
}
```

| Field | Type | Description |
|-------|------|-------------|
| `id` | `int` | Unique identifier |
| `name` | `string` | Location name |
| `type` | `string` | Type of location (e.g., "Planet", "Space station") |
| `dimension` | `string` | Dimension name (e.g., "Dimension C-137") |
| `residents` | `string[]` | Array of character URLs for residents |
| `url` | `string` | URL to location's own endpoint |
| `created` | `string` | ISO 8601 timestamp of creation in database |

### Episode Schema

**Total Episodes:** 51

```typescript
interface Episode {
  id: number;
  name: string;
  air_date: string;
  episode: string;
  characters: string[];
  url: string;
  created: string;
}
```

| Field | Type | Description |
|-------|------|-------------|
| `id` | `int` | Unique identifier |
| `name` | `string` | Episode title |
| `air_date` | `string` | Air date (e.g., "December 2, 2013") |
| `episode` | `string` | Episode code in format `S##E##` |
| `characters` | `string[]` | Array of character URLs appearing in episode |
| `url` | `string` | URL to episode's own endpoint |
| `created` | `string` | ISO 8601 timestamp of creation in database |

---

## API Endpoints

### Character Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/character` | Get all characters (paginated) |
| GET | `/character/{id}` | Get single character by ID |
| GET | `/character/{id1},{id2},...` | Get multiple characters by IDs |
| GET | `/character/?{filters}` | Get filtered characters |

**Filter Parameters:**

| Parameter | Type | Description |
|-----------|------|-------------|
| `name` | `string` | Filter by character name (partial match) |
| `status` | `string` | Filter by status: `alive`, `dead`, `unknown` |
| `species` | `string` | Filter by species |
| `type` | `string` | Filter by type/subspecies |
| `gender` | `string` | Filter by gender: `female`, `male`, `genderless`, `unknown` |

**Examples:**

```bash
# Get all characters
GET /character

# Get character with ID 2
GET /character/2

# Get characters with IDs 1, 183, and 2
GET /character/1,183,2

# Filter by name and status
GET /character/?name=rick&status=alive

# Combine multiple filters
GET /character/?species=Human&gender=male&status=alive
```

### Location Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/location` | Get all locations (paginated) |
| GET | `/location/{id}` | Get single location by ID |
| GET | `/location/{id1},{id2},...` | Get multiple locations by IDs |
| GET | `/location/?{filters}` | Get filtered locations |

**Filter Parameters:**

| Parameter | Type | Description |
|-----------|------|-------------|
| `name` | `string` | Filter by location name (partial match) |
| `type` | `string` | Filter by location type |
| `dimension` | `string` | Filter by dimension name |

**Examples:**

```bash
# Get all locations
GET /location

# Get location with ID 3
GET /location/3

# Get multiple locations
GET /location/3,21

# Filter by name and type
GET /location/?name=Earth&type=Planet

# Filter by dimension
GET /location/?dimension=Dimension C-137
```

### Episode Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/episode` | Get all episodes (paginated) |
| GET | `/episode/{id}` | Get single episode by ID |
| GET | `/episode/{id1},{id2},...` | Get multiple episodes by IDs |
| GET | `/episode/?{filters}` | Get filtered episodes |

**Filter Parameters:**

| Parameter | Type | Description |
|-----------|------|-------------|
| `name` | `string` | Filter by episode name (partial match) |
| `episode` | `string` | Filter by episode code (e.g., `S01E01`) |

**Examples:**

```bash
# Get all episodes
GET /episode

# Get episode with ID 28
GET /episode/28

# Get multiple episodes
GET /episode/10,28

# Filter by name
GET /episode/?name=Pilot

# Filter by episode code
GET /episode/?episode=S01E01
```

---

## Entity Relationships

The following diagram illustrates the relationships between entities:

```
┌─────────────────────────────────────────────────────────────────┐
│                                                                 │
│  ┌───────────┐         origin          ┌────────────┐          │
│  │           │ ◄─────────────────────  │            │          │
│  │  Location │                         │  Character │          │
│  │           │ ◄─────────────────────  │            │          │
│  └───────────┘       location          └────────────┘          │
│       │                                      │                 │
│       │ residents                   episode  │                 │
│       │ (URLs to Characters)        (URLs)   │                 │
│       ▼                                      ▼                 │
│  ┌───────────┐                        ┌────────────┐           │
│  │ Character │                        │  Episode   │           │
│  │  (many)   │ ◄───────────────────── │            │           │
│  └───────────┘      characters        └────────────┘           │
│                     (URLs to Characters)                       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Relationship Details

| From | To | Field | Type | Description |
|------|----|-------|------|-------------|
| Character | Location | `origin` | Reference | Character's origin location |
| Character | Location | `location` | Reference | Character's last known location |
| Character | Episode | `episode` | URL Array | Episodes where character appears |
| Location | Character | `residents` | URL Array | Characters residing at location |
| Episode | Character | `characters` | URL Array | Characters appearing in episode |

### Relationship Cardinality

```
Character ─────┬───── N:1 ────── Location (origin)
              │
              └───── N:1 ────── Location (current)

Character ─────────── N:M ────── Episode
              (via episode[] and characters[])

Location ──────────── 1:N ────── Character (residents)
```

### Resolving Relationships

All relationships are represented as URLs. To resolve related entities:

```typescript
// Example: Get a character and resolve their origin location
const character = await fetch('/api/character/1').then(r => r.json());
// character.origin = { name: "Earth (C-137)", url: "https://rickandmortyapi.com/api/location/1" }

// Resolve the origin location
const origin = await fetch(character.origin.url).then(r => r.json());

// Example: Get all characters from an episode
const episode = await fetch('/api/episode/1').then(r => r.json());
// episode.characters = ["https://rickandmortyapi.com/api/character/1", ...]

// Extract IDs and fetch multiple characters at once
const ids = episode.characters.map(url => url.split('/').pop()).join(',');
const characters = await fetch(`/api/character/${ids}`).then(r => r.json());
```

---

## Response Examples

### Character Response

```json
{
  "id": 1,
  "name": "Rick Sanchez",
  "status": "Alive",
  "species": "Human",
  "type": "",
  "gender": "Male",
  "origin": {
    "name": "Earth (C-137)",
    "url": "https://rickandmortyapi.com/api/location/1"
  },
  "location": {
    "name": "Citadel of Ricks",
    "url": "https://rickandmortyapi.com/api/location/3"
  },
  "image": "https://rickandmortyapi.com/api/character/avatar/1.jpeg",
  "episode": [
    "https://rickandmortyapi.com/api/episode/1",
    "https://rickandmortyapi.com/api/episode/2"
  ],
  "url": "https://rickandmortyapi.com/api/character/1",
  "created": "2017-11-04T18:48:46.250Z"
}
```

### Location Response

```json
{
  "id": 1,
  "name": "Earth (C-137)",
  "type": "Planet",
  "dimension": "Dimension C-137",
  "residents": [
    "https://rickandmortyapi.com/api/character/38",
    "https://rickandmortyapi.com/api/character/45"
  ],
  "url": "https://rickandmortyapi.com/api/location/1",
  "created": "2017-11-10T12:42:04.162Z"
}
```

### Episode Response

```json
{
  "id": 1,
  "name": "Pilot",
  "air_date": "December 2, 2013",
  "episode": "S01E01",
  "characters": [
    "https://rickandmortyapi.com/api/character/1",
    "https://rickandmortyapi.com/api/character/2"
  ],
  "url": "https://rickandmortyapi.com/api/episode/1",
  "created": "2017-11-10T12:56:33.798Z"
}
```

---

## Error Handling

The API returns standard HTTP status codes:

| Status Code | Description |
|-------------|-------------|
| 200 | Success |
| 404 | Resource not found |
| 500 | Internal server error |

Error response format:

```json
{
  "error": "Character not found"
}
```

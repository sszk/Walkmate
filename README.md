# Walkmate

## English

Walkmate is a Pebble watch face for tracking daily steps.

### Overview

- Displays the current date and time
- Focuses on showing today's step count
- Draws a progress ring based on a step goal
- Step goal is configurable
- Uses Pebble health features to fetch step data

### Key Files

- `src/c/Walkmate.c` - Main application logic
- `package.json` - Project configuration
- `build/appinfo.json` - Pebble app settings
- `resources/` - Media assets such as images and fonts

### Usage

#### Build

Run from the project root with Pebble SDK installed:

```sh
pebble build
```

#### Install

If a Pebble device is connected:

```sh
pebble install --phone <device-ip-address>
```

Or install directly to the watch:

```sh
pebble install
```

### Customization

- The step goal can be configured via `APP_KEY_STEP_GOAL`.
- Default step goal is `10000`.

### Supported Platforms

- Aplite
- Basalt
- Chalk
- Diorite
- Emery
- Flint
- Gabbro

### License

This project is licensed under the BSD 3-Clause License.

```text
Copyright (c) 2026, Shinsuke Suzuki
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

## 日本語

Walkmate は Pebble 向けの歩数トラッキングウォッチフェイスです。

### 概要

- 現在の日付と時刻を表示
- 今日の歩数を中心に表示する環境
- 目標歩数に基づく進捗リングを描画
- 目標歩数は設定可能
- Pebble 健康機能を使用して歩数データを取得

### 主要ファイル

- `src/c/Walkmate.c` - メインアプリケーションロジック
- `package.json` - プロジェクト定義
- `build/appinfo.json` - Pebble アプリ設定
- `resources/` - 画像やフォントなどのメディアリソース

### 使い方

#### ビルド

Pebble SDK がインストールされている環境で、プロジェクトルートから実行します。

```sh
pebble build
```

#### インストール

Pebble デバイスが接続されている場合:

```sh
pebble install --phone <デバイスのIPアドレス>
```

または、ウォッチ側に直接インストールする場合:

```sh
pebble install
```

### カスタマイズ

- 目標歩数は `APP_KEY_STEP_GOAL` を通じて設定できます。
- デフォルトの目標歩数は `10000` です。

### サポートプラットフォーム

- Aplite
- Basalt
- Chalk
- Diorite
- Emery
- Flint
- Gabbro

### ライセンス

このプロジェクトは BSD 3-Clause License のもとで公開されています。

```text
Copyright (c) 2026, Shinsuke Suzuki
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

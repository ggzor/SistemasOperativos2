import * as React from "react"
import { render } from "react-dom"
import styled, { css } from "styled-components"

import { ComputerCard } from "./components/ComputerCard"
import { useState } from "react"
import { useEffect } from "react"
import { LoadingIndicator } from "./components/LoadingIndicator"
import { useReducer } from "react"

window.receiver = null
window.receive = function (message) {
  if (receiver) receiver(message)
}

const dump = (obj) => JSON.stringify(obj, null, 0)

const App = ({}) => {
  const [state, setState] = useState({
    phase: "initial",
    computers: [],
  })

  const [selection, dispatchSelection] = useReducer(
    ({ folderSelections, fileSelections }, action) => {
      let newFileSelection = {}

      switch (action.type) {
        case "UPDATED_STATE":
          for (let c of state.computers)
            if (fileSelections[c.hash]) {
              const hashes = allHashes(c.fileSystem)
              const deleting = deletingNodes(c.fileSystem)
              newFileSelection[c.hash] = fileSelections[c.hash].filter(
                (x) => hashes.includes(x) && !deleting.includes(x)
              )
            } else newFileSelection[c.hash] = []
          break
        case "TOGGLE_FILE":
          if (fileSelections[action.computer].includes(action.file.hash))
            newFileSelection = {
              ...fileSelections,
              [action.computer]: fileSelections[action.computer].filter(
                (hash) => hash != action.file.hash
              ),
            }
          else
            newFileSelection = {
              ...fileSelections,
              [action.computer]: [
                action.file.hash,
                ...fileSelections[action.computer],
              ],
            }
          break
        case "TOGGLE_DIRECTORY":
          if (folderSelections[action.computer].includes(action.file.hash))
            newFileSelection = {
              ...fileSelections,
              [action.computer]: fileSelections[action.computer].filter(
                (hash) => !hash.startsWith(action.file.hash)
              ),
            }
          else {
            const computerFS = state.computers.find(
              (c) => c.hash === action.computer
            ).fileSystem
            const deleting = deletingNodes(computerFS)

            newFileSelection = {
              ...fileSelections,
              [action.computer]: [
                ...fileSelections[action.computer].filter(
                  (hash) => !hash.startsWith(action.file.hash)
                ),
                ...allHashes(action.file).filter((x) => !deleting.includes(x)),
              ],
            }
          }
          break
        default:
          break
      }

      const newFolderSelection = {}
      for (let computerName in newFileSelection) {
        const computerFS = state.computers.find((c) => c.hash === computerName)
          .fileSystem

        newFolderSelection[computerName] = computeFolderSelections(
          computerFS,
          newFileSelection[computerName],
          deletingNodes(computerFS)
        )[2]
      }

      return {
        folderSelections: newFolderSelection,
        fileSelections: newFileSelection,
      }
    },
    { folderSelections: {}, fileSelections: {} }
  )

  useEffect(() => dispatchSelection({ type: "UPDATED_STATE" }), [state])

  useEffect(() => {
    window.receiver = setState
    return () => (window.receiver = null)
  })

  function handleCardEvent(event) {
    if (event.type == "click") {
      if (event.file.type === "file")
        dispatchSelection({ ...event, type: "TOGGLE_FILE" })
      else dispatchSelection({ ...event, type: "TOGGLE_DIRECTORY" })
    } else if (event.type == "receive") {
      const toSend = {}
      for (let k in selection.fileSelections)
        if (selection.fileSelections[k].length > 0)
          toSend[k] = selection.fileSelections[k]

      sendMessage({
        type: "send",
        target: event.computer,
        items: toSend,
      })
    }
  }

  const globalSelection = []
  for (let k in selection.fileSelections)
    globalSelection.push(...selection.fileSelections[k])

  function handleEvent(event) {
    if (event.type === "delete" && globalSelection.length > 0) {
      const toSend = {}
      for (let k in selection.fileSelections)
        if (selection.fileSelections[k].length > 0)
          toSend[k] = selection.fileSelections[k]

      sendMessage({ type: "delete", items: toSend })
    }
  }

  const sendMessage = (event) => {
    console.log(JSON.stringify({ type: "message", message: event }))
  }

  return (
    <AppLayout
      {...state}
      onKeyDown={(e) =>
        e.key === "Delete" ? handleEvent({ type: "delete" }) : undefined
      }
      tabIndex={-1}
    >
      {state.phase !== "running" && (
        <CentralMessage>
          {state.phase === "initial"
            ? "Conectando con el servidor central"
            : state.phase === "named"
            ? "Esperando otros clientes"
            : ""}
        </CentralMessage>
      )}
      {state.phase === "running" &&
        state.computers.map((c) => (
          <ComputerCard
            key={c.hash}
            selection={[
              ...(selection.fileSelections[c.hash] || []),
              ...(selection.folderSelections[c.hash] || []),
            ]}
            globalSelection={globalSelection}
            {...c}
            onEvent={handleCardEvent}
          />
        ))}
    </AppLayout>
  )
}

const cssForPhase = {
  initial: css`
    align-items: center;
    justify-content: center;
    height: 100vh;
  `,
  running: css`
    margin: 4px;
    flex-wrap: wrap;

    & > * {
      margin: 12px;
    }
  `,
}

const AppLayout = styled.div`
  display: flex;
  outline: 0;

  ${({ phase }) =>
    phase === "running" ? cssForPhase[phase] : cssForPhase["initial"]}
`

const CentralMessageLayout = styled.section`
  display: flex;
  flex-direction: column;
  align-items: center;
  user-select: none;

  font-size: 24px;
  text-align: center;
  color: rgba(0, 0, 0, 0.7);
`

const CentralMessage = ({ children }) => {
  return (
    <CentralMessageLayout>
      {children}
      <LoadingIndicator />
    </CentralMessageLayout>
  )
}

function allHashes(fs) {
  if (fs) {
    if (fs.type === "file") return [fs.hash]
    else return fs.files.flatMap((f) => allHashes(f))
  }
  return []
}

function deletingNodes(fs) {
  if (fs) {
    if (fs.type === "file") {
      return fs.state === "deleting" ? [fs.hash] : []
    } else {
      return fs.files.flatMap((f) => deletingNodes(f))
    }
  } else {
    return []
  }
}

function computeFolderSelections(fs, fileSelections, deleting) {
  if (fs) {
    if (fs.type === "file") {
      return [fileSelections.includes(fs.hash), deleting.includes(fs.hash), []]
    } else {
      let isSelected = true
      let isDeleting = true
      let selectedDirs = []

      for (let subf of fs.files) {
        const [
          isSubSelected,
          isSubDeleting,
          newSelected,
        ] = computeFolderSelections(subf, fileSelections, deleting)

        isSelected &= isSubSelected
        isDeleting &= isSubDeleting
        selectedDirs = [...selectedDirs, ...newSelected]
      }

      const missingToSelect = allHashes(fs).filter(
        (x) => !fileSelections.includes(x) && !deleting.includes(x)
      )
      const partiallySelected = missingToSelect.length === 0

      return [
        (isSelected || partiallySelected) && !isDeleting,
        isDeleting,
        (isSelected || partiallySelected) && !isDeleting
          ? [fs.hash, ...selectedDirs]
          : selectedDirs,
      ]
    }
  } else {
    return []
  }
}

render(<App />, document.getElementById("app"))
